#include "Motor.h"
#include "freertos/task.h"
#include <stdio.h>

Driver1::Driver1(gpio_num_t nmt1, gpio_num_t nmt2, gpio_num_t dir1, gpio_num_t dir2){
    PWM1 = nmt1;
    PWM2 = nmt2;
    DIR1 = dir1;
    DIR2 = dir2;

    PWM1_STATE = false, PWM2_STATE = false, DIR1_STATE = false, DIR2_STATE  = false;
    //Resetear pines al inicializar la clase
    gpio_reset_pin(PWM1), gpio_reset_pin(PWM2), gpio_reset_pin(DIR1), gpio_reset_pin(DIR2);
    
    //Configurar los pines para solo salida
    gpio_set_direction(PWM1, GPIO_MODE_OUTPUT), gpio_set_direction(PWM2, GPIO_MODE_OUTPUT), gpio_set_direction(DIR1, GPIO_MODE_OUTPUT), gpio_set_direction(DIR2, GPIO_MODE_OUTPUT);
   
}

void Driver1::motor_1_toggle(void){
    PWM1_STATE = !PWM1_STATE;
    gpio_set_level(PWM1, PWM1_STATE);
    printf("Motor on pin %d toggled to state %d\n", PWM1, PWM1_STATE);
}

void Driver1::motor_2_toggle(void){
    PWM2_STATE = !PWM2_STATE;
    gpio_set_level(PWM2, PWM2_STATE);
    printf("Motor on pin %d toggled to state %d\n", PWM2, PWM2_STATE);
}

void Driver1::motor1_direction_toggle(void){
    DIR1_STATE = !DIR1_STATE;
    gpio_set_level(DIR1, DIR1_STATE);
    printf("Motor direction on pin %d toggled to state %d\n", DIR1, DIR1_STATE);
}

void Driver1::motor2_direction_toggle(void){
    DIR2_STATE = !DIR2_STATE;
    gpio_set_level(DIR2, DIR2_STATE);
    printf("Motor direction on pin %d toggled to state %d\n", DIR2, DIR2_STATE);
}

