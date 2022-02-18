#include "pico/stdlib.h"
#include "6847pi.h"

const uint32_t SLEEP = 5000;

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

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
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