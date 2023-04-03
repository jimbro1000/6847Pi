#ifndef PALLETTE_H
#define PALLETTE_H

// rgb values
const int rgb_palette[3 * 12] = {
    0,7,0, //green
    7,7,0, //yellow
    1,0,5, //blue
    5,0,1, //red
    7,7,7, //buff
    0,6,4, //cyan
    7,0,7, //magenta
    7,2,0, //orange
    0,0,0, //black
    0,2,0, //green-black
    2,1,0, //orange-black
    7,5,2,  //bright-orange
};

// yab values
const int yab_palette[3 * 12] = {
    4,0,0, //green
    3,0,1, //yellow
    5,2,1, //blue
    5,1,2, //red
    3,1,1, //buff
    4,1,1, //cyan
    4,2,2, //magenta
    4,0,2, //orange
    5,1,1, //black
    5,0,0, //green-black
    5,0,2, //orange-black
    4,0,2, //bright-orange
};

const int graphics_css0_palette[4] = {
    0,1,2,3,
};

const int graphics_css1_palette[4] = {
    4,5,6,7,
};

const int text_css0_palette[10] = {
    8,0,1,2,3,4,5,6,7,9,
};

const int text_css1_palette[10] = {
    8,11,1,2,3,4,5,6,7,10,
};

const int semi4_palette[9] = {
    8,0,1,2,3,4,5,6,7,
};

const int semi6_css0_palette[5] = {
    8,0,1,2,3,
};

const int semi6_css1_palette[5] = {
    8,4,5,6,7,
};

const int luma_css0_palette[2] = {
    8,0,
};

const int luma_css1_palette[2] = {
    8,4,
};

#endif
