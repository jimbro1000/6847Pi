#ifndef FONT_DEF_H
#define FONT_DEF_H

#define CHAR_ROW_MAX 12
#define CHAR_COL_MAX 64

extern const int mc6847_font[];

uint8_t get_character_row(uint8_t character, uint8_t row);
uint8_t get_semi4_block(uint8_t row, uint8_t col);
uint8_t get_semi6_block(uint8_t row, uint8_t col);

#endif