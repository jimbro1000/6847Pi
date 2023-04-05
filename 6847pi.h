#include "palette.h"

#ifndef ANALOG_6847_OUTPUT_H
#define ANALOG_6847_OUTPUT_H

// const int Y_PINS[3] = {31,32,34};
// const int Y_SCALE = 3;
// const int Y_MAX = 5;
// const int Y_MIN = 0;
// const int Y_MAP[6] = {0,1,3,4,6,7};
// const int B_PINS[2] = {26,27};
// const int B_SCALE = 2;
// const int B_MAX = 2;
// const int B_MIN = 0;
// const int B_MAP[3] = {0,2,3};
// const int A_PINS[2] = {24,25};
// const int A_SCALE = 2;
// const int A_MAX = 3;
// const int A_MIN = 0;
// const int A_MAP[4] = {0,1,2,3};

const int RGB_SCALE = 9;
const int RGB_PINS[9] = {22,24,25,26,27,29,31,32,34};

#define DEFAULT_ROW_SIZE 256
#define SEMI_ROW_SIZE 256
#define WIDE_ROW_SIZE 320
#define OUTPUT_BUFFER_SIZE 12

const double CLOCK = 25175000L;
const double PIXEL_INTERVAL = 1/CLOCK;

const int DEFAULT_FRONT_PORCH = 16;
const int DEFAULT_SYNC = 96;
const int DEFAULT_BACK_PORCH = 48;

const int LOW_SCREEN_X = 320;
const int LOW_SCREEN_Y = 240;
const int VGA_RATIO = 2;

struct OutputRow {
    struct OutputRow *next;
    uint16_t row_size;
    uint16_t row[WIDE_ROW_SIZE];
};

struct SourceDataState {
    uint8_t data;
    bool external;
    bool inverse;
    bool semigraphics;
    bool graphics;
    bool colour_set;
    uint8_t graphics_mode;
    uint8_t text_row;
};

struct PixelValue {
    uint8_t palette_index;
    uint8_t pixel_width;
    struct Palette palette;
    struct PixelValue *next;
};

#endif

#ifndef CLOCK_6847_INTERFACE
#define CLOCK_6847_INTERFACE
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