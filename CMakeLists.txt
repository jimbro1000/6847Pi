cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(6847pi)
pico_sdk_init()

add_executable(6847pi
        src/6847pi.c
        src/palette.c
        src/font.c
        src/output.c
        src/rowBuffer.c
        src/pinInterface.c
        src/vga.c)

target_link_libraries(6847pi pico_stdlib pico_multicore)

pico_add_extra_outputs(6847pi)
