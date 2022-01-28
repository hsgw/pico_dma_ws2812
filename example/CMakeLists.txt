cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico_sdk_import.cmake)
project(ws2812_simple)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(${PROJECT_NAME}
    main.cpp
)

add_subdirectory(../ pico_dma_ws2812)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ../
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    pico_stdlib
    pico_multicore
    pico_dma_ws2812
)

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})