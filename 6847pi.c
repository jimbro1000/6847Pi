#include "pico/stdlib.h"
#include "6847pi.h"

int main() {
    init_analog_output(Y_PIN0, Y_SCALE);
    init_analog_output(B_PIN0, B_SCALE);
    init_analog_output(A_PIN0, A_SCALE);

    uint a = A_MIN;
    uint b = B_MIN;
    uint y = Y_MIN;
    while (true) {
        a = a++ % (A_MAX + 1);
        b = b++ % (B_MAX + 1);
        y = y++ % (Y_MAX + 1);
        analog_output(a, A_PIN0, A_SCALE, A_MAP);
        analog_output(b, B_PIN0, B_SCALE, B_MAP);
        analog_output(y, Y_PIN0, Y_SCALE, Y_MAP);
    }
}

void init_analog_output(int pin0, int scale) {
    for (int i = 0; i < scale; ++i) {
        gpio_init(pin0 + i);
        gpio_set_dir(pin0 + i, GPIO_OUT);
    }
    reset_analog_output(pin0, scale);
}

void reset_analog_output(int pin0, int scale) {
    for (int i = 0; i < scale; ++i) {
        gpio_put(pin0 + i, 1);
    }
}

void analog_output(int value, int pin0, int scale, int[] value_map) {
    reset_analog_output(pin0, scale);
    if (value < len(value_map)) {
        int output = value_map[value];
        for (int i = 0; i < scale; ++i) {
            gpio_put(pin0 + i, output % 2);
            output = output / 2;
        }
    }
}
