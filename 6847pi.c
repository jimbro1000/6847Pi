#include "pico/stdlib.h"
#include "6847pi.h"
#include "font.h"
#include "palette.h"

const uint32_t SLEEP = 5000;
const uint32_t DEFAULT_ROW_BYTES = 32;
const uint32_t BUFFER_SLEEP = 1;

#ifndef CLOCK_6847_INTERFACE
    const uint32_t READ_DELAY = 5; //ms
    const uint8_t EXT_PIN = 11; //GPIO8
    const uint8_t INV_PIN = 9; //GPIO6
    const uint8_t SEMI_PIN = 10; //GPIO7
    const uint8_t GRAPHICS_PIN = 19; //GPIO14
    const uint8_t COLOUR_SET = 22; //GPIO17
    const uint8_t GM0_PIN = 11; // GPIO8
    const uint8_t GM1_PIN = 12; //GPIO9
    const uint8_t GM2_PIN = 14; //GPIO10
    const uint8_t DATA_PIN_START = 1; //GPIO0
    const uint8_t DATA_PIN_COUNT = 8; //data bus width
    const uint8_t CLOCK_PIN = 20; //GPIO15
    const uint8_t DA0_PIN = 22; //GPIO17
    const uint8_t HS_PIN = 21; //GPIO16
    const uint8_t FS_PIN = 17; //GPIO13
#endif

// output buffer
struct OutputRow row_pipe[OUTPUT_BUFFER_SIZE];
// buffer insert point
struct OutputRow *row_pipe_ptr;
// buffer output point
struct OutputRow *read_pipe_ptr;
uint32_t current_row_size = DEFAULT_ROW_BYTES;
uint8_t row_counter = 0;

/**
 * Initialise output buffering.
 */
void init_row_pipe() {
    // link output buffer rows
    row_pipe[OUTPUT_BUFFER_SIZE - 1].next = &row_pipe[0]; 
    for (int i = OUTPUT_BUFFER_SIZE - 2; i >= 0; --i) {
        row_pipe[i].next = &row_pipe[i+1];
    }
    // set insert and output pointers
    row_pipe_ptr = &row_pipe[0];
    read_pipe_ptr = &row_pipe[0];
    // set screen line byte length
    for (int i = 0; i < OUTPUT_BUFFER_SIZE; ++i) {
        row_pipe_ptr->row_size = current_row_size;
        for (int j = 0; j < current_row_size; ++j) {
            row_pipe_ptr->row[j] = 0;
        }
        row_pipe_ptr = row_pipe_ptr->next;
    }
}

/**
 * Reset output levels on DAC pins.
 *
 * @param pins array of pins to reset
 * @param scale number of pins to reset (default 3)
 */
// bitbang manual output
// this needs transferring to PIO state machine
void reset_analog_output(const int pins[], int scale) {
    for (int i = 0; i < scale; ++i) {
        gpio_put(pins[i], 0);
    }
}

/**
 * Initialise DAC output pins.
 *
 * @param pins array of pins to reset
 * @param scale number of pins to reset (default 3)
 */
void init_analog_output(const int pins[], int scale) {
    for (int i = 0; i < scale; ++i) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
    }
    reset_analog_output(pins, scale);
}

/**
 * Write RGB values to DAC output pins.
 *
 * @param value RGB value
 * @param pins array of output pins
 * @param scale number of pins in array
 */
void analog_output(int value, const int pins[], int scale) {
    for (int i = 0; i < scale; ++i) {
        gpio_put(pins[i], value && 1);
        value = value >> 1;
    }
}

/**
 * Put formatted pixel output row to output buffer.
 *
 * accepts input only if space exists in buffer
 *
 * @param new_row output row
 * @return success
 */
bool push_to_output_buffer(struct OutputRow *new_row) {
    bool result = false;
    if (row_pipe_ptr->next != read_pipe_ptr) {
        row_pipe_ptr->row_size = new_row->row_size;
        for (int j=0; j<new_row->row_size; ++j) {
            row_pipe_ptr->row[j] = new_row->row[j];
        }
        result = true;
        row_pipe_ptr = row_pipe_ptr->next;
    }
    return result;
}

struct OutputRow pop_from_output_buffer() {
    struct OutputRow result;
    result.row_size = read_pipe_ptr->row_size;
    for (int j=0; j<read_pipe_ptr->row_size; ++j) {
        result.row[j] = read_pipe_ptr->row[j];
    }
    return result;
}

/**
 * Convert pixel data into formatted RGB word (3 bits per value masked $01FF).
 *
 * @param pixel structured pixel data with embedded palette reference
 * @return RGB word
 */
uint16_t pixel_to_rgb(struct PixelValue pixel) {
    uint16_t palette_index = pixel.palette.refs[pixel.palette_index] * 3;
    uint16_t result = (pixel.palette.source[palette_index] << 6) +
                      (pixel.palette.source[palette_index + 1] << 3) +
                      (pixel.palette.source[palette_index + 2]);
    return result;
}

/**
 * Append RGB data to output row from block of pixel values.
 *
 * Expands wide pixels to multiple entries of the same colour
 *
 * @param output RGB output row
 * @param index tail index of output row
 * @param pixel head of structured pixel data linked list
 * @return revised tail index of output row
 */
uint8_t pixel_block_to_rgb_row(struct OutputRow *output, uint8_t index, struct PixelValue *pixel) {
    struct PixelValue *pixelPtr = pixel;
    while (pixelPtr != NULL) {
        for (int i=0; i < pixel->pixel_width; ++i) {
            output->row[index++] = pixel_to_rgb(*pixel);
        }
        pixelPtr = pixelPtr->next;
    }
    return index;
}

/**
 * Convert raw binary graphic data into list of pixel data.
 *
 * @param source source data byte
 * @param bpp bits per pixel in source data
 * @param palette applicable palette mapping for pixel data
 * @return forward linked list of pixel data
 */
struct PixelValue extract_graphics_pixel(uint8_t source, uint8_t bpp, struct Palette palette) {
    struct PixelValue results[8];
    struct PixelValue result = results[0];
    int ppb = 8 / bpp;
    for (int j = 0; j < ppb; ++j) {
        struct PixelValue current = results[j];
        int pixel = 0;
        for (int i = 0; i < bpp; ++i) {
            if ((source & 128) != 0) {
                pixel += 1;
            }
            source = (source << 1) & 255;
            pixel = (pixel << 1) & 255;
        }
        current.palette_index = pixel;
        current.palette = palette;
        if (j>0) {
            results[j-1].next = &current;
        }
    }
    return result;
}

/**
 * Convert source data into semi-graphics 4 pixel data.
 *
 * @param source source data byte
 * @param character_row character row counter
 * @param palette applicable colour palette
 * @return head of linked list of structured pixel data
 */
struct PixelValue extract_semigraphics4_pixel(uint8_t source, uint8_t character_row, struct Palette palette) {
    struct PixelValue results[8];
    struct PixelValue result = results[0];
    for (int j = 0; j < 8; ++j) {
        struct PixelValue current = results[j];
        current.pixel_width = 1;
        int bit;
        if (j < 4) {
            bit = 1;
        } else {
            bit = 2;
        }
        if (character_row < 6) {
            bit = bit << 2;
        }
        bool pixel_set = (source & bit) != 0;
        current.palette = palette;
        if (pixel_set == 0) {
            current.palette_index = 1 + ((source & 112) >> 4);
        } else {
            current.palette_index = 0;
        }
        if (j > 0) {
            results[j-1].next = &current;
        }
    }
    return result;
}

/**
 * Convert source data into semi-graphics 6 pixel data.
 *
 * @param source source data byte
 * @param character_row character row counter
 * @param palette applicable colour palette
 * @return head of linked list of structured pixel data
 */
struct PixelValue extract_semigraphics6_pixel(uint8_t source, uint8_t character_row, struct Palette palette) {
    struct PixelValue results[8];
    struct PixelValue result = results[0];
    for (int j = 0; j < 8; ++j) {
        struct PixelValue current = results[j];
        current.pixel_width = 1;
        int bit;
        if (j < 4) {
            bit = 1;
        } else {
            bit = 2;
        }
        if (character_row < 8) {
            bit = bit << 2;
            if (character_row < 4) {
                bit = bit << 2;
            }
        }
        bool pixel_set = (source & bit) != 0;
        current.palette = palette;
        if (pixel_set == 0) {
            current.palette_index = 1 + ((source & 192) >> 6);
        } else {
            current.palette_index = 0;
        }
        if (j > 0) {
            results[j-1].next = &current;
        }
    }
    return result;
}

/**
 * Checked push of row data to output buffer.
 *
 * Blocks until row can be accepted
 *
 * @param row output row data
 */
void safe_push_row(struct OutputRow *row) {
    bool accepted = false;
    do {
        accepted = push_to_output_buffer(row);
        if (!accepted) {
            sleep_ms(BUFFER_SLEEP);
        }
    } while (!accepted);
}

/**
 * Convert sampled text input into a screen row of RGB pixel data.
 *
 * Text input also covers semigraphics to enable characters above 127
 *
 * @param source_buffer single row of sampled screen data
 * @param buffer_size buffer run length (default 32)
 * @param row_ratio number of rows to generate from the same sampled input (default 12)
 * @param bpp bits per pixel
 */
void generate_text_rows(struct SourceDataState *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp) {
    const uint16_t cycles = 8;
    const uint16_t row_size = buffer_size * cycles;
    struct PixelValue pixelHead;
    for (int i = 0; i < row_ratio; ++i) {
        struct OutputRow row;
        row.row_size = row_size;
        int counter = 0;
        for (int j = 0; j < buffer_size; ++j) {
            uint8_t data = source_buffer[j]->data;
            uint8_t text_row = source_buffer[j]->text_row;
            if (data > 127) {
                struct Palette palette = select_palette(source_buffer[j]->colour_set,
                                                        source_buffer[j]->semigraphics,
                                                        source_buffer[j]->graphics,
                                                        source_buffer[j]->external);
                //semigraphics
                if (source_buffer[j]->external) {
                    //SG4
                    pixelHead = extract_semigraphics4_pixel(data,
                                                            text_row,
                                                            palette);
                } else {
                    //SG6
                    pixelHead = extract_semigraphics6_pixel(data,
                                                            text_row,
                                                            palette);
                }
            } else {
                uint8_t source = get_character_row(data, text_row);
                struct Palette palette = select_palette(source_buffer[j]->colour_set,
                                                        source_buffer[j]->semigraphics,
                                                        source_buffer[j]->graphics,
                                                        source_buffer[j]->external);
                pixelHead = extract_graphics_pixel(source, bpp, palette);
            }
            counter = pixel_block_to_rgb_row(&row, counter, &pixelHead);
        }
        safe_push_row(&row);
    }
}

/**
 * Convert sampled graphic input into a screen row of RGB pixel data.
 * 
 * @param source_buffer sampled source data
 * @param buffer_size buffer size
 * @param row_ratio display rows per source row
 * @param bpp bits per pixel
 * @param palette row palette
 */
void generate_graphic_rows(struct SourceDataState *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp, struct Palette palette) {
    const uint16_t cycles = 8 / bpp;
    const uint16_t row_size = buffer_size * cycles;
    struct PixelValue pixelHead;
    int counter = 0;
    for (int i = 0; i < row_ratio; ++i) {
        struct OutputRow row;
        row.row_size = row_size;
        for (int j = 0; j < buffer_size; ++j) {
            uint8_t source = source_buffer[j]->data;
            pixelHead = extract_graphics_pixel(source, bpp, palette);
        }
        counter = pixel_block_to_rgb_row(&row, counter, &pixelHead);
        safe_push_row(&row);
    }
}

/**
 * Generate row data from source data buffer.
 *
 * @param source_buffer sampled source data
 * @param buffer_size buffer run length
 * @param row_ratio screen rows per source row
 * @param bpp bits per pixel
 */
void generate_row(struct SourceDataState *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp) {
    struct SourceDataState primary = *source_buffer[0];
    if (primary.graphics) {
        struct Palette palette = select_palette(primary.colour_set, primary.semigraphics, primary.graphics, primary.external);
        generate_graphic_rows(source_buffer, buffer_size, row_ratio, bpp, palette);
    } else {
        generate_text_rows(source_buffer, buffer_size, 1, 8);
    }
}

/**
 * Sample one byte of data from input.
 *
 * Returns data in referenced input data structure
 *
 * @param sample sample data structure to populate
 */
void sample_data(struct SourceDataState *sample) {
    // sample control lines
    sample->external = gpio_get(EXT_PIN);
    sample->inverse = gpio_get(INV_PIN);
    sample->semigraphics = gpio_get(SEMI_PIN);
    sample->graphics = gpio_get(GRAPHICS_PIN);
    sample->colour_set = gpio_get(COLOUR_SET);
    sample->graphics_mode = 0;
    if (gpio_get(GM2_PIN)) {
        sample->graphics_mode += 4;
    }
    if (gpio_get(GM1_PIN)) {
        sample->graphics_mode += 2;
    }
    if (gpio_get(GM0_PIN)) {
        sample->graphics_mode += 1;
    }
    // sample data bus
    uint8_t data_pin = DATA_PIN_START;
    sample->data = 0;
    for (int i = 0; i < DATA_PIN_COUNT; ++i) {
        if (gpio_get(data_pin)) {
            sample->data += 128;
        }
        sample->data = sample->data >> 1;
    }
    sample->text_row = row_counter;
}

/**
 * Sample full row of source data from inputs.
 *
 * @param buffer_size length of available buffer
 */
void sample_row_data(uint8_t buffer_size) {
    struct SourceDataState *source_buffer[buffer_size];
    //synchronise reads with cpu timing using graphics clock
    //wait for clock_pin rising edge
    bool clock_state = false;
    bool da0 = false;
    for (int i = 0; i < buffer_size; ++i) {
        while (!clock_state) {
            clock_state = gpio_get(CLOCK_PIN);
        }
        // set DA0
        gpio_put(DA0_PIN, da0);
        da0 = !da0;
        // sleep while data settles
        sleep_ms(READ_DELAY);
        sample_data(source_buffer[i]);
        while (clock_state) {
            clock_state = gpio_get(CLOCK_PIN);
        }
    }
    generate_row(source_buffer, buffer_size, VGA_RATIO, 8);
}

/**
 * Generate a row of video data from buffer.
 * 
 * Current implementation is almost definitely too slow and blocking
*/
void generate_video_row(struct OutputRow output) {
    gpio_put(HS_PIN, 1);
    for (int j=0; j<DEFAULT_BACK_PORCH; ++j) {
        analog_output(0, RGB_PINS, RGB_SCALE);
        reset_analog_output(RGB_PINS, RGB_SCALE);
    }
    for (int j=0; j<output.row_size; ++j) {
        analog_output(output.row[j], RGB_PINS, RGB_SCALE);
        reset_analog_output(RGB_PINS, RGB_SCALE);
    }
    for (int j=0; j<DEFAULT_FRONT_PORCH; ++j) {
        analog_output(0, RGB_PINS, RGB_SCALE);
        reset_analog_output(RGB_PINS, RGB_SCALE);
    }
    gpio_put(HS_PIN, 0);
}

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    init_row_pipe();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    #ifdef ANALOG_6847_OUTPUT_H
        init_analog_output(RGB_PINS, RGB_SCALE);
        while (true) {
            sample_row_data(32);
            reset_analog_output(RGB_PINS, RGB_SCALE);
            gpio_put(LED_PIN, 0);
            sleep_ms(SLEEP);
            generate_video_row(pop_from_output_buffer());
            // analog_output(value, RGB_PINS, RGB_SCALE);
            gpio_put(LED_PIN, 1);
            sleep_ms(SLEEP);
        }
    #else
        gpio_put(LED_PIN, 0);
        sleep_ms(300);
        gpio_put(LED_PIN, 1);
        sleep_ms(300)
    #endif
}