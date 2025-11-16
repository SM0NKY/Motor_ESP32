#ifndef MOTOR_H
#define MOTOR_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

class Motor {
    public:
        Motor(gpio_num_t pin);
        void toggle(void);

    public:
        gpio_num_t pin;
        bool state;      
};

#endif