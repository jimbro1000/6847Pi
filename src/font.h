#ifndef FONT_DEF_H
#define FONT_DEF_H

#define CHAR_ROW_MAX 12
#define CHAR_COL_MAX 64
#define FONT_LENGTH = (CHAR_COL_MAX * CHAR_ROW_MAX)

    extern const int mc6847_font[];
    extern const int semi4_block[];
    extern const int semi6_block[];

    uint8_t get_character_row(uint8_t character, uint8_t row);

#endif