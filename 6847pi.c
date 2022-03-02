#include "pico/stdlib.h"
#include "6847pi.h"

const uint32_t SLEEP = 5000;
const uint32_t OUTPUT_BUFFER_SIZE = 12;
const unit32_t DEFAULT_ROW_SIZE = 32;
const uint32_t BUFFER_SLEEP = 1;

row_pipe OutputRow[OUTPUT_BUFFER_SIZE];
OutputRow *row_pipe_ptr;
OutputRow *read_pipe_ptr;
uint32_t current_row_size = DEFAULT_ROW_SIZE;

void init_row_pipe() {
    row_pipe[OUTPUT_BUFFER_SIZE - 1].next = &row_pipe[0]; 
    for (int i = OUTPUT_BUFFER_SIZE - 2; i >= 0; --i) {
        row_pipe[i] = &row_pipe[i+1];
    }
    row_pipe_ptr = &row_pipe[0];
    read_pipe_ptr = &row_pipe[0];
    for (int i = 0; i < OUTPUT_BUFFER_SIZE; ++i) {
        row_pipe_ptr->row_size = current_row_size;
        for (int j = 0; j < current_row_size; ++j) {
            row_pipe_ptr->row[j] = 0;
        }
        row_pipe_ptr = row_pipe_ptr->next;
        row_pipe_ptr->bpp = 1;
    }
}

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

bool push_to_output_buffer(OutputRow *new_row) {
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

void generate_graphic_rows(uint8_t *source_buffer[], uint8_t buffer_size, uint8_t row_ratio, uint8_t bpp, uint8_t palette) {
    const uint16_t cycles = 8 / bpp;
    const uint16_t row_size = buffer_size * cycles;
    for (int i = 0; i < row_ratio; ++i) {
        OutputRow row;
        row.row_size = row_size;
        row.bpp = bpp;
        row.palette = palette;
        int counter = 0;
        for (int j = 0; j < buffer_size; ++j) {
            uint8_t source = source_buffer[j];
            for (int k = 0; k < cycles; ++k) {
                uint16_t pixel = 0;
                for (int l = 0; l < bpp; ++l) {
                    if (source && 256) {
                        ++pixel;
                    }
                    source = source << 1;
                }
                row[counter++] = pixel;
            }
        }
        bool accepted = false;
        do {
            accepted = push_to_output_buffer(&row);
            if (!accepted) {
                sleep_ms(BUFFER_SLEEP);
            }
        } until (accepted == true);
    }
}

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    output_buffer = output_buffer[OUTPUT_BUFFER_SIZE];
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