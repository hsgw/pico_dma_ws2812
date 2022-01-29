// Copyright (c) 2022, Takuya Urakawa (Dm9Records 5z6p.com)
// SPDX-License-Identifier: MIT

// Original copyright
// License: MIT
// Copyright (c) 2021 Pimoroni Ltd
// https://github.com/pimoroni/pimoroni-pico/tree/main/drivers/plasma
// Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
// License: BSD-3-Clause

#pragma once

#include <cstdint>
#include <math.h>

#include "ws2812.pio.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"
#include "pico/mutex.h"

class WS2812 {
   public:
    static constexpr uint SERIAL_FREQ_400KHZ  = 400000;
    static constexpr uint SERIAL_FREQ_800KHZ  = 800000;
    static constexpr uint DEFAULT_SERIAL_FREQ = SERIAL_FREQ_800KHZ;
    static constexpr uint LED_RESET_TIME      = 400;

    static constexpr uint BUFFER_COUNT = 2;
    static constexpr uint BUFFER_IN    = 0;
    static constexpr uint BUFFER_OUT   = 1;

#pragma pack(push, 1)
    union alignas(4) GRB {
        struct {
            uint8_t g;
            uint8_t r;
            uint8_t b;
        };
        uint32_t srgb;
        void operator=(uint32_t v) { srgb = v; };
        void rgb(uint8_t r, uint8_t g, uint8_t b) {
            this->g = g;
            this->r = r;
            this->b = b;
        };
        GRB() : r(0), g(0), b(0){};
    };
#pragma pack(pop)

    WS2812(uint num_leds, PIO pio, uint sm, uint pin,
           uint freq = DEFAULT_SERIAL_FREQ);
    ~WS2812() {
        clear();
        update(true);
        dma_channel_unclaim(dma_channel);
        pio_sm_set_enabled(pio, sm, false);
        pio_remove_program(pio, &ws2812_program, pio_program_offset);
#ifndef MICROPY_BUILD_TYPE
        // pio_sm_unclaim seems to hardfault in MicroPython
        pio_sm_unclaim(pio, sm);
#endif
        // Only delete buffers we have allocated ourselves.
        for (uint i = 0; i < BUFFER_COUNT; i++) {
            delete[] buffer[i];
        }
        delete[] buffer;
    }

    static int dma_channel;
    volatile static mutex_t data_send_mutex;

    static int64_t reset_time_alert_callback(alarm_id_t id, void *user_data);
    static void dma_complete_callback();

    bool update(bool blocking = false);
    void send();
    void clear();
    void set_hsv(uint32_t index, float h, float s, float v);
    void set_rgb(uint32_t index, uint8_t r, uint8_t g, uint8_t b);

   private:
    PIO pio;
    uint sm;
    uint pio_program_offset;
    uint32_t num_leds;
    GRB **buffer;
    bool request_send = false;
    mutex_t flip_buffer_mutex;
};