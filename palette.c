#include "pico/stdlib.h"
#include "palette.h"

const int* text_palette(bool colour_set) {
    if (colour_set) {
        return text_css1_palette;
    }
    return text_css0_palette;
}

const int* semi_palette(bool colour_set) {
    if (colour_set) {
        return semi6_css1_palette;
    }
    return semi6_css0_palette;
}

const int* graphics_palette(bool colour_set) {
    if (colour_set) {
        return graphics_css1_palette;
    }
    return graphics_css0_palette;
}

const int* luma_palette(bool colour_set) {
    if (colour_set) {
        return luma_css1_palette;
    }
    return luma_css0_palette;
}

struct Palette select_palette(bool colour_set, bool semigraphics, bool graphics, bool external) {
    struct Palette result;
    result.source = rgb_palette;
    if (!semigraphics && !graphics) {
        result.palette_length = 2;
        result.refs = text_palette(colour_set);
    } else if (semigraphics && !external) {
        result.palette_length = 9;
        result.refs = semi4_palette;
    } else if (semigraphics) {
        result.palette_length = 5;
        result.refs = semi_palette(colour_set);
    } else {
        if (graphics % 2 == 0) {
            // colour graphics
            result.palette_length = 4;
            result.refs = graphics_palette(colour_set);
        } else {
            // luma graphics
            result.palette_length = 2;
            result.refs = luma_palette(colour_set);
        }
    }
    return result;
}