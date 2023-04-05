#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "6847pi.h"
#include "rowBuffer.h"

//
// Created by Julian on 05/04/2023.
//

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
        gpio_put(pins[i], value & 1);
        value = value >> 1;
    }
}

/**
 * Generate a row of video data from buffer.
 *
 * Current implementation is almost definitely too slow and blocking
*/
void generate_video_row(struct output_row output) {
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

void core1_render() {
    init_analog_output(RGB_PINS, RGB_SCALE);
    reset_analog_output(RGB_PINS, RGB_SCALE);
    while(true) {
        generate_video_row(pop_from_output_buffer());
    }
}
