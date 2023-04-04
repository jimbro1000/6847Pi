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
#endif

// output buffer
struct OutputRow row_pipe[OUTPUT_BUFFER_SIZE];
// buffer insert point
struct OutputRow *row_pipe_ptr;
// buffer output point
struct OutputRow *read_pipe_ptr;
uint32_t current_row_size = DEFAULT_ROW_BYTES;
uint8_t row_counter = 0;

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
        row_pipe_ptr->bpp = 1;
    }
}

// manual output
// this needs transferring to PIO state machine
void reset_analog_output(const int pins[], int scale) {
    for (int i = 0; i < scale; ++i) {
        gpio_put(pins[i], 0);
    }
}

void init_analog_output(const int pins[], int scale) {
    for (int i = 0; i < scale; ++i) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
    }
    reset_analog_output(pins, scale);
}

void analog_output(int value, const int pins[], int scale, int max, const int value_map[]) {
    //reset_analog_output(pin0, scale);
    if (value < max) {
        int output = value_map[value];
        for (int i = 0; i < scale; ++i) {
            gpio_put(pins[i], output % 2);
            output = output / 2;
        }
    }
}

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

void safe_push_row(struct OutputRow *row) {
    bool accepted = false;
    do {
        accepted = push_to_output_buffer(row);
        if (!accepted) {
            sleep_ms(BUFFER_SLEEP);
        }
    } while (accepted);
}

void generate_text_rows(struct SourceDataState *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp) {
    const uint16_t cycles = 8;
    const uint16_t row_size = buffer_size * cycles;
    for (int i = 0; i < row_ratio; ++i) {
        struct OutputRow row;
        row.row_size = row_size;
        row.bpp = bpp;
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
                    row.row[counter++] = extract_semigraphics4_pixel(data,
                                                                     text_row,
                                                                     palette);
                } else {
                    //SG6
                    row.row[counter++] = extract_semigraphics6_pixel(data,
                                                                     text_row,
                                                                     palette);
                }
            } else {
                uint8_t source = get_character_row(data, text_row);
                for (int k = 0; k < cycles; ++k) {
                    struct Palette palette = select_palette(source_buffer[j]->colour_set,
                                                            source_buffer[j]->semigraphics,
                                                            source_buffer[j]->graphics,
                                                            source_buffer[j]->external);
                    row.row[counter++] = extract_graphics_pixel(source, bpp, palette);
                }
            }
        }
        safe_push_row(&row);
    }
}

void generate_graphic_rows(struct SourceDataState *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp, struct Palette palette) {
    const uint16_t cycles = 8 / bpp;
    const uint16_t row_size = buffer_size * cycles;
    for (int i = 0; i < row_ratio; ++i) {
        struct OutputRow row;
        row.row_size = row_size;
        row.bpp = bpp;
        row.palette = palette;
        int counter = 0;
        for (int j = 0; j < buffer_size; ++j) {
            uint8_t source = source_buffer[j]->data;
            for (int k = 0; k < cycles; ++k) {
                row.row[counter++] = extract_graphics_pixel(source, bpp, palette);
            }
        }
        safe_push_row(&row);
    }
}

void generate_row(struct SourceDataState *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp) {
    struct SourceDataState primary = *source_buffer[0];
    if (primary.graphics) {
        struct Palette palette = select_palette(primary.colour_set, primary.semigraphics, primary.graphics, primary.external);
        generate_graphic_rows(source_buffer, buffer_size, row_ratio, bpp, palette);
    } else {
        generate_text_rows(source_buffer, buffer_size, 1, 8);
    }
}

void sample_data(struct SourceDataState *sample) {
    // need to provide tick
    // wait for data to settle
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

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    init_row_pipe();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    #ifdef ANALOG_6847_OUTPUT_H
        init_analog_output(Y_PINS, Y_SCALE);
        init_analog_output(B_PINS, B_SCALE);
        init_analog_output(A_PINS, A_SCALE);

        uint a = A_MIN;
        uint b = B_MIN;
        uint y = Y_MIN;
        while (true) {
//            sample_row_data(32);
            reset_analog_output(A_PINS, A_SCALE);
            reset_analog_output(B_PINS, B_SCALE);
            reset_analog_output(Y_PINS, Y_SCALE);
            a = (a + 1) % (A_MAX + 1);
            b = (b + 1) % (B_MAX + 1);
            y = (y + 1) % (Y_MAX + 1);
            gpio_put(LED_PIN, 0);
            sleep_ms(SLEEP);
            analog_output(a, A_PINS, A_SCALE, A_MAX, A_MAP);
            analog_output(b, B_PINS, B_SCALE, B_MAX, B_MAP);
            analog_output(y, Y_PINS, Y_SCALE, Y_MAX, Y_MAP);
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