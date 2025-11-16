#ifndef BLINKER_H
#define BLINKER_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

class Blinker {
    public:
        Blinker(gpio_num_t pin);
        void blink(int intervals_ms);

    public:
        gpio_num_t pin;
        bool state;      
};

#endif // BLINKER_H