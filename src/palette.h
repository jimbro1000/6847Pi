#ifndef PALETTE_H
#define PALETTE_H

#define PALETTE_SIZE 36

typedef struct palette {
    int palette_length;
    const int *source;
    const int *refs;
} palette_t;

// rgb values
extern const int rgb_palette[PALETTE_SIZE];
// yab values
extern const int yab_palette[PALETTE_SIZE];

extern const int text_css0_palette[];
extern const int text_css1_palette[];
extern const int semi4_palette[];
extern const int semi6_css0_palette[];
extern const int semi6_css1_palette[];
extern const int luma_css0_palette[];
extern const int luma_css1_palette[];
extern const int graphics_css0_palette[];
extern const int graphics_css1_palette[];

palette_t select_palette(bool colour_set, bool semigraphics, bool graphics, bool external);

#endif //PALETTE_H
