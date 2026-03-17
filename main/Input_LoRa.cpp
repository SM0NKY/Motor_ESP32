#include "Input_LoRa.h"

InputLoRa::InputLoRa(gpio_num_t scl, gpio_num_t sda, i2c_port_t master, uint32_t frequency){
    this->scl = scl;
    this->sda = sda;
    this->master = master;
    this->frequency = frequency;
    
    this->InitI2C();
}

void InputLoRa::InitI2C(){
    i2c_config_t i2c_conf;
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = this->sda;
    i2c_conf.scl_io_num = this->scl;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = this->frequency;
    i2c_param_config(this->master, &i2c_conf);
    i2c_driver_install(this->master, I2C_MODE_MASTER, 0, 0, 0);
}

void InputLoRa::read_data(){
    LoRADATA_t buffer;

    esp_err_t ret = i2c_master_read_from_device(
        this->master, 
        0x08, // Dirección del dispositivo LoRa (ejemplo)
        (uint8_t*)&buffer, 
        sizeof(LoRADATA_t), 
        pdMS_TO_TICKS(50)
    );

    if (ret == ESP_OK) {
        this->x = buffer.x; //Guardado en int16_t
        this->y = buffer.y; //Guardado en int16_t
        this->brakes = buffer.brakes; //Guardado en uint8_t
    }
    return ret;
}