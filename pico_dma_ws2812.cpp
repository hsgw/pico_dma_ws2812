// Copyright (c) 2022, Takuya Urakawa (Dm9Records 5z6p.com)
// SPDX-License-Identifier: MIT

#include "pico_dma_ws2812.hpp"

#include <cstring>
#include "stdio.h"

int WS2812::dma_channel;
volatile mutex_t WS2812::data_send_mutex;

WS2812::WS2812(uint num_leds, PIO pio, uint sm, uint pin, uint freq)
    : num_leds(num_leds), pio(pio), sm(sm) {
    pio_program_offset = pio_add_program(pio, &ws2812_program);

    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

    pio_sm_config c = ws2812_program_get_default_config(pio_program_offset);
    sm_config_set_sideset_pins(&c, pin);

    sm_config_set_out_shift(&c, false, true, 24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
    float div          = clock_get_hz(clk_sys) / (freq * cycles_per_bit);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, pio_program_offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    dma_channel               = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(dma_channel);
    channel_config_set_bswap(&config, true);
    channel_config_set_dreq(&config, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&config, DMA_SIZE_32);
    channel_config_set_read_increment(&config, true);
    dma_channel_set_trans_count(dma_channel, num_leds, false);
    dma_channel_set_read_addr(dma_channel, (uint32_t *)buffer[BUFFER_OUT],
                              false);
    dma_channel_configure(dma_channel, &config, &pio->txf[sm], NULL, 0, false);

    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_complete_callback);
    irq_set_enabled(DMA_IRQ_0, true);

    mutex_init(const_cast<mutex_t *>(&data_send_mutex));
    mutex_init(&flip_buffer_mutex);

    buffer = new GRB *[BUFFER_COUNT];
    for (uint i = 0; i < BUFFER_COUNT; i++) {
        buffer[i] = new GRB[num_leds];
    }
}

bool WS2812::update(bool blocking) {
    if (!request_send) return false;

    bool mutex_owned =
        mutex_try_enter(const_cast<mutex_t *>(&data_send_mutex), NULL);

    if (!mutex_owned && !blocking) return false;

    if (!mutex_owned)
        mutex_enter_blocking(const_cast<mutex_t *>(&data_send_mutex));

    // copy buffer to out
    mutex_enter_blocking(&flip_buffer_mutex);
    memcpy(buffer[BUFFER_OUT], buffer[BUFFER_IN], sizeof(GRB) * num_leds);
    request_send = false;
    mutex_exit(&flip_buffer_mutex);

    add_alarm_in_us(LED_RESET_TIME, reset_time_alert_callback, (void *)this,
                    false);

    if (blocking) {
        while (dma_channel_is_busy(dma_channel)) {
        }
    }
    return true;
}

void WS2812::send() {
    dma_channel_set_trans_count(dma_channel, num_leds, false);
    dma_channel_set_read_addr(dma_channel, buffer[BUFFER_OUT], true);
}

int64_t WS2812::reset_time_alert_callback(alarm_id_t id, void *user_data) {
    WS2812 *ws2812 = static_cast<WS2812 *>(user_data);
    ws2812->send();
    return 0;
}

void WS2812::dma_complete_callback() {
    mutex_exit(const_cast<mutex_t *>(&data_send_mutex));
    dma_hw->ints0 = 1u << WS2812::dma_channel;
}

void WS2812::clear() {
    for (auto i = 0u; i < num_leds; ++i) {
        set_rgb(i, 0, 0, 0);
    }
}

void WS2812::set_hsv(uint32_t index, float h, float s, float v) {
    float i = floor(h * 6.0f);
    float f = h * 6.0f - i;
    v *= 255.0f;
    uint8_t p = v * (1.0f - s);
    uint8_t q = v * (1.0f - f * s);
    uint8_t t = v * (1.0f - (1.0f - f) * s);

    switch (int(i) % 6) {
        case 0:
            set_rgb(index, v, t, p);
            break;
        case 1:
            set_rgb(index, q, v, p);
            break;
        case 2:
            set_rgb(index, p, v, t);
            break;
        case 3:
            set_rgb(index, p, q, v);
            break;
        case 4:
            set_rgb(index, t, p, v);
            break;
        case 5:
            set_rgb(index, v, p, q);
            break;
    }
}

void WS2812::set_rgb(uint32_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= num_leds) return;

    mutex_enter_blocking(&flip_buffer_mutex);
    buffer[BUFFER_IN][index].rgb(r, g, b);
    request_send = true;
    mutex_exit(&flip_buffer_mutex);
}
