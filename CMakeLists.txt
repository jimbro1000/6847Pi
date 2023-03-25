cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(6847pi)
pico_sdk_init()

add_executable(6847pi
        6847pi.c
        )

target_link_libraries(6847pi pico_stdlib)

pico_add_extra_outputs(6847pi)
