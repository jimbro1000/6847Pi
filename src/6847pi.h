#ifndef ANALOG_6847_OUTPUT_H
#define ANALOG_6847_OUTPUT_H

#include "palette.h"

#define RGB_SCALE 9
const int RGB_PINS[RGB_SCALE] = {22,24,25,26,27,29,31,32,34};

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

struct output_row {
    struct output_row *next;
    uint16_t row_size;
    uint16_t row[WIDE_ROW_SIZE];
};

struct source_data_state {
    uint8_t data;
    bool external;
    bool inverse;
    bool semigraphics;
    bool graphics;
    bool colour_set;
    uint8_t graphics_mode;
    uint8_t text_row;
};

struct pixel_value {
    uint8_t palette_index;
    uint8_t pixel_width;
    struct palette palette;
    struct pixel_value *next;
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