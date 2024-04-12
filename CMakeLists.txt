cmake_minimum_required(VERSION 3.13)

include(/usr/local/raspberrypi-pico-sdk/pico_sdk_init.cmake)

project(am2315)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(am2315
        am2315.cpp
	libam2315.cpp
        )

# pull in common dependencies
target_link_libraries(am2315
	pico_stdlib
	pico_malloc
	hardware_i2c
	)

add_compile_options(
	-Wall
	-Wno-format
	-Wno-unused-function
	-Wno-maybe-uninitialized
	)

pico_enable_stdio_usb(am2315 1)
pico_enable_stdio_uart(am2315 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(am2315)
