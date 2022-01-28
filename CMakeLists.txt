add_library(pico_dma_ws2812 INTERFACE)

pico_generate_pio_header(pico_dma_ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_sources(pico_dma_ws2812 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pico_dma_ws2812.cpp
)

target_include_directories(pico_dma_ws2812 INTERFACE
${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(pico_dma_ws2812 INTERFACE
    pico_stdlib hardware_pio hardware_dma
)