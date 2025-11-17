#ifndef MOTOR_H
#define MOTOR_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

class Driver1 {
    public:
        Driver1(gpio_num_t PWM1, gpio_num_t PWM2, gpio_num_t DIR1, gpio_num_t DIR2);
        
        // Activar o desactivar motores
        void motor_1_toggle(void);
        void motor_2_toggle(void);
        
        //Cambiar el giro de los motores
        void motor1_direction_toggle(void);
        void motor2_direction_toggle(void);

    public:
        //Pines para darle corriente los motores 
        gpio_num_t PWM1, PWM2;
        bool PWM1_STATE, PWM2_STATE;

        //Pines para cambio de giro
        gpio_num_t DIR1, DIR2;
        bool DIR1_STATE, DIR2_STATE;


              
};

#endif