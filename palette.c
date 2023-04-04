#include "pico/stdlib.h"
#include "palette.h"

struct Palette select_palette(bool colour_set, bool semigraphics, bool graphics, bool external, int mode) {
    struct Palette result;
    result.source = &rgb_palette;
    if (!semigraphics && !graphics) {
        result.palette_length = 2;
        if (colour_set) {
            result.refs = &text_css1_palette;
        } else {
            result.refs = &text_css0_palette;
        }
    } else if (semigraphics && !external) {
        result.palette_length = 9;
        result.refs = &semi4_palette;
    } else if (semigraphics && external) {
        result.palette_length = 5;
        if (colour_set) {
            result.refs = &semi6_css1_palette;
        } else {
            result.refs = &semi6_css0_palette;
        }
    } else {
        switch (graphics) {
            case 0, 2, 4, 6: // colour graphics
                result.palette_length = 4;
                if (colour_set) {
                    result.refs = &graphics_css1_palette;
                } else {
                    result.refs = &graphics_css0_palette;
                }
                break;
            default: // luma graphics
                result.palette_length = 2;
                if (colour_set) {
                    result.refs = &luma_css1_palette;
                } else {
                    result.refs = &luma_css0_palette;
                }
                break;
        }
    }
    return result;
};