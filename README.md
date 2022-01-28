C++ WS2812 Driver for the raspberry pi pico
===============================
C++ simple WS2812 driver using pico hardware DMA and timer.

based on pimoroni's plasma library.   
https://github.com/pimoroni/pimoroni-pico/tree/main/drivers/plasma

## Usage

### initialize 
```c++
WS2812(uint num_leds, PIO pio, uint sm, uint pin, uint freq = DEFAULT_SERIAL_FREQ, GRB *buffer = nullptr)
```

### write led
```c++
set_rgb(uint32_t index, uint8_t r, uint8_t g, uint8_t b)
```

### send data to leds  
This should be called when leds are updated or repeat in as short time as possible.   
If set `blocking = true` and transmission is in progress, this function will wait until the end of it.
```c++
update(bool blocking = false)
```

## Example
`example` contains threading example.