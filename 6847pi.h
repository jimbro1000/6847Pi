#ifndef ANALOG_6847_OUTPUT_H
#define ANALOG_6847_OUTPUT_H

const int Y_PINS[3] = {31,32,34};
const int Y_SCALE = 3;
const int Y_MAX = 5;
const int Y_MIN = 0;
const int Y_MAP[6] = {0,1,3,4,6,7};
const int B_PINS[2] = {26,27};
const int B_SCALE = 2;
const int B_MAX = 2;
const int B_MIN = 0;
const int B_MAP[3] = {0,2,3};
const int A_PINS[2] = {24,25};
const int A_SCALE = 2;
const int A_MAX = 3;
const int A_MIN = 0;
const int A_MAP[4] = {0,1,2,3};

const int DEFAULT_ROW_SIZE = 256;
const int SEMI_ROW_SIZE = 256;
const int WIDE_ROW_SIZE = 320;
const int DEFAULT_FRONT_PORCH = 16;
const int DEFAULT_SYNC = 96;
const int DEFAULT_BACK_PORCH = 48;

const int LOW_SCREEN_X = 320;
const int LOW_SCREEN_Y = 240;
const int VGA_RATIO = 2;

struct OutputRow {
    OutputRow *next;
    uint16_t row_size;
    uint16_t row[WIDE_ROW_SIZE];
    uint8_t bpp;
    uint8_t palette;
}

#endif