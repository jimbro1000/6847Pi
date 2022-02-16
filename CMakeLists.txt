cmake_minimum_required(VERSION 3.16)

project(6847Pi)

add_executable(6847Pi
        6847pi.c
        )

target_link_libraries(6847pi pico_stdlib)

pico_add_extra_outputs(6847pi)