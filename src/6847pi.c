#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "6847pi.h"
#include "font.h"
#include "palette.h"
#include "rowBuffer.h"
#include "output.h"
#include "pinInterface.h"
#include "vga.h"

uint8_t row_counter;

/**
 * Convert pixel data into formatted RGB word (3 bits per value masked $01FF).
 *
 * @param pixel structured pixel data with embedded palette reference
 * @return RGB word
 */
uint16_t pixel_to_rgb(pixel_value_t pixel) {
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
uint8_t pixel_block_to_rgb_row(output_row_t *output, uint8_t index, pixel_value_t *pixel) {
    pixel_value_t *pixelPtr = pixel;
    while (pixelPtr != NULL) {
        for (int i = 0; i < pixel->pixel_width; ++i) {
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
pixel_value_t extract_graphics_pixel(uint8_t source, uint8_t bpp, palette_t palette) {
    pixel_value_t results[8];
    pixel_value_t result = results[0];
    int ppb = 8 / bpp;
    for (int j = 0; j < ppb; ++j) {
        pixel_value_t current = results[j];
        int pixel = 0;
        for (int i = 0; i < bpp; ++i) {
            if ((source & 0x80) != 0) {
                pixel += 1;
            }
            source = (source << 1) & 0xFF;
            pixel = (pixel << 1) & 0xFF;
        }
        current.palette_index = pixel;
        current.palette = palette;
        if (j > 0) {
            results[j - 1].next = &current;
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
pixel_value_t extract_semigraphics4_pixel(uint8_t source, uint8_t character_row, palette_t palette) {
    pixel_value_t results[8];
    pixel_value_t result = results[0];
    for (int j = 0; j < 8; ++j) {
        pixel_value_t current = results[j];
        current.pixel_width = 1;
        int bit = get_semi4_block(character_row, j);
        bool pixel_set = (source & bit) != 0;
        current.palette = palette;
        if (pixel_set == 0) {
            current.palette_index = 1 + ((source & 0x70) >> 4);
        } else {
            current.palette_index = 0;
        }
        if (j > 0) {
            results[j - 1].next = &current;
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
pixel_value_t extract_semigraphics6_pixel(uint8_t source, uint8_t character_row, palette_t palette) {
    pixel_value_t results[8];
    pixel_value_t result = results[0];
    for (int j = 0; j < 8; ++j) {
        pixel_value_t current = results[j];
        current.pixel_width = 1;
        int bit = get_semi6_block(character_row, j);
        bool pixel_set = (source & bit) != 0;
        current.palette = palette;
        if (pixel_set == 0) {
            current.palette_index = 1 + ((source & 0xC0) >> 6);
        } else {
            current.palette_index = 0;
        }
        if (j > 0) {
            results[j - 1].next = &current;
        }
    }
    return result;
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
void generate_text_rows(source_data_state_t *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp) {
    const uint16_t cycles = 8;
    const uint16_t row_size = buffer_size * cycles;
    struct pixel_value pixelHead;
    for (int i = 0; i < row_ratio; ++i) {
        output_row_t row;
        row.row_size = row_size;
        int counter = 0;
        for (int j = 0; j < buffer_size; ++j) {
            uint8_t data = source_buffer[j]->data;
            uint8_t text_row = source_buffer[j]->text_row;
            if (data > 0x7F) {
                struct palette palette = select_palette(source_buffer[j]->colour_set,
                                                        source_buffer[j]->semigraphics,
                                                        source_buffer[j]->graphics,
                                                        source_buffer[j]->external);
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
                struct palette palette = select_palette(source_buffer[j]->colour_set,
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
void generate_graphic_rows(source_data_state_t *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp,
                           palette_t palette) {
    const uint16_t cycles = 8 / bpp;
    const uint16_t row_size = buffer_size * cycles;
    pixel_value_t pixelHead;
    int counter = 0;
    for (int i = 0; i < row_ratio; ++i) {
        output_row_t row;
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
void generate_row(source_data_state_t *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp) {
    source_data_state_t primary = *source_buffer[0];
    if (primary.graphics) {
        palette_t palette = select_palette(primary.colour_set, primary.semigraphics,
                                           primary.graphics, primary.external);
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
void sample_data(source_data_state_t *sample) {
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
            sample->data += 0x80;
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
    source_data_state_t *source_buffer[buffer_size];
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
    row_counter = (row_counter + 1) % 12;
    generate_row(source_buffer, buffer_size, VGA_RATIO, 8);
}

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    init_row_pipe();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    row_counter = 0;

#ifdef ANALOG_6847_OUTPUT_H
    /// \tag::setup_multicore[]
    multicore_launch_core1(core1_render);
    /// \end::setup_multicore[]
    while (true) {
        sample_row_data(32);
        gpio_put(LED_PIN, 0);
        sleep_ms(SLEEP);
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