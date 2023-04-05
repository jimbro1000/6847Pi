#include "pico/stdlib.h"
#include "font.h"

uint8_t get_character_row(uint8_t character, uint8_t row) {
    if (character >= char_col_max || row >= char_row_max) {
        return 0;
    }
    return _mc6847_font[character * char_row_max + row];
}