#include "pico/stdlib.h"
//
// Created by julian on 06/04/23.
//
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

const int RGB_SCALE = 9;
const int RGB_PINS[9] = {22,24,25,26,27,29,31,32,34};
