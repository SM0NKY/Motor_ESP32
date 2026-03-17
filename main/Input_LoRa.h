#pragma once
#ifndef INPUT_LORA_H
#define INPUT_LORA_H

#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

class InputLoRa {
    public:
        InputLoRa(gpio_num_t scl, gpio_num_t sda, i2c_port_t master, uint32_t frequency);
        
        void read_data();

    private:
        gpio_num_t scl;
        gpio_num_t sda;
        i2c_port_t master;
        uint32_t frequency;

        uint16_t x,y;
        uint8_t brakes; // Para uso posterior

        void InitI2C();
        
};

#endif // INPUT_LORA_H