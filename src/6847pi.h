#ifndef ANALOG_6847_OUTPUT_H
#define ANALOG_6847_OUTPUT_H

#include "palette.h"

#define DEFAULT_ROW_SIZE 256
#define SEMI_ROW_SIZE 256
#define WIDE_ROW_SIZE 320
#define OUTPUT_BUFFER_SIZE 12

typedef struct output_row output_row_t;
struct output_row {
    output_row_t *next;
    uint16_t row_size;
    uint16_t row[WIDE_ROW_SIZE];
};

typedef struct {
    uint8_t data;
    bool external;
    bool inverse;
    bool semigraphics;
    bool graphics;
    bool colour_set;
    uint8_t graphics_mode;
    uint8_t text_row;
} source_data_state_t;

typedef struct pixel_value pixel_value_t;
struct pixel_value {
    uint8_t palette_index;
    uint8_t pixel_width;
    struct palette palette;
    pixel_value_t *next;
};

#endif
