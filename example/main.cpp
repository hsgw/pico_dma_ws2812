// Copyright (c) 2022, Takuya Urakawa (Dm9Records 5z6p.com)
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "../pico_dma_ws2812.hpp"

// for xiao rp2040
// static constexpr uint LED_POWER_PIN = 11;
// static constexpr uint LED_DATA_PIN  = 12;

static constexpr uint LED_DATA_PIN = 3;
static constexpr uint LED_NUM      = 8;

WS2812 leds(LED_NUM, pio0, 0, LED_DATA_PIN);

void core1_entry() {
    while (true) {
        leds.update(false);
    }
}

int main() {
    stdio_init_all();
    // gpio_init(LED_POWER_PIN);
    // gpio_set_dir(LED_POWER_PIN, GPIO_OUT);
    // gpio_put(LED_POWER_PIN, 1);

    multicore_launch_core1(core1_entry);

    printf("start\n");

    bool color = true;
    while (1) {
        // printf("update %d\n", color ? 0 : 1);
        for (int i = 0; i < LED_NUM; i++) {
            leds.set_rgb(i, !color ? 10 : 0, 0, color ? 10 : 0);
        }
        color = !color;
        sleep_ms(500);
    }
    return 0;
}