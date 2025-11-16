#include "Motor.h"
#include "freertos/task.h"
#include <stdio.h>

Motor::Motor(gpio_num_t numpin){
    pin = numpin;
    state = false;

    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

void Motor::toggle(void){
    state = !state;
    gpio_set_level(pin, state);
    printf("Motor on pin %d toggled to state %d\n", pin, state);
}