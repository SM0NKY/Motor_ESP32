#include "Motor.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

//Definir variables para el control de la velocidad de los motores
#define MOTOR_FREQ 5000 //Frecuencia del Duty Cycle 
#define MOTOR_RESOLUTION LEDC_TIMER_8_BIT //Resolucion de 8 bits (0-255)
#define MOTOR_MODE LEDC_LOW_SPEED_MODE //Modo de baja frecuencia
#define TIMER LEDC_TIMER_0  //Indicar cual timer se va a utilizar [0,1,2,3]

//Se utiliza el constructor para inicializar los pines y las variables
Driver1::Driver1(gpio_num_t nmt1, gpio_num_t nmt2, gpio_num_t dir1, gpio_num_t dir2){
    PWM1 = nmt1;
    PWM2 = nmt2;
    DIR1 = dir1;
    DIR2 = dir2;

    motor1_speed = 0.0, motor2_speed = 0, DIR1_STATE = false, DIR2_STATE  = false;
    //Resetear pines al inicializar la clase
    gpio_reset_pin(PWM1), gpio_reset_pin(PWM2), gpio_reset_pin(DIR1), gpio_reset_pin(DIR2);
    
    //Configurar los pines para solo salida
    gpio_set_direction(PWM1, GPIO_MODE_OUTPUT), gpio_set_direction(PWM2, GPIO_MODE_OUTPUT), gpio_set_direction(DIR1, GPIO_MODE_OUTPUT), gpio_set_direction(DIR2, GPIO_MODE_OUTPUT);
   
    //Configracion del timer para el PWM 
    ledc_timer_config_t timer_config;
    memset(&timer_config, 0, sizeof(ledc_timer_config_t));

    timer_config.speed_mode = MOTOR_MODE;
    timer_config.duty_resolution = MOTOR_RESOLUTION;
    timer_config.timer_num = TIMER;
    timer_config.freq_hz = MOTOR_FREQ;
    timer_config.clk_cfg = LEDC_AUTO_CLK;

    ledc_timer_config(&timer_config);

    //Configuracion de los canales para cada motor

        // -- Canal motor 1 -- //
    ledc_channel_config_t channel1_config;
    memset(&channel1_config, 0, sizeof(ledc_channel_config_t));
    channel1_config.gpio_num = PWM1;
    channel1_config.speed_mode = MOTOR_MODE;
    channel1_config.channel = LEDC_CHANNEL_0;
    //channel1_config.intr_type = LEDC_INTR_DISABLE;
    channel1_config.timer_sel = TIMER;
    channel1_config.duty = 0;
    channel1_config.hpoint = 0;

    ledc_channel_config(&channel1_config);

        // -- Canal motor 2 -- //
    ledc_channel_config_t channel2_config;
    memset(&channel2_config, 0, sizeof(ledc_channel_config_t));

    channel2_config.gpio_num = PWM2;
    channel2_config.speed_mode = MOTOR_MODE;
    channel2_config.channel = LEDC_CHANNEL_1;
    //channel2_config.intr_type = LEDC_INTR_DISABLE;
    channel2_config.timer_sel = TIMER;
    channel2_config.duty = 0;
    channel2_config.hpoint = 0;
    
    ledc_channel_config(&channel2_config);
}



// void Driver1::motor_1_toggle(void){
//     if (SPEED_CONTROL == false){    
//         PWM1_STATE = !PWM1_STATE;
//         gpio_set_level(PWM1, PWM1_STATE);
//         printf("Motor on pin %d toggled to state %d\n", PWM1, PWM1_STATE);
//     }
// }

// void Driver1::motor_2_toggle(void){
//     PWM2_STATE = !PWM2_STATE;
//     gpio_set_level(PWM2, PWM2_STATE);
//     printf("Motor on pin %d toggled to state %d\n", PWM2, PWM2_STATE);
// }
float Driver1::motor_speed_to_dutyCycle(float speed){

    if (speed < 0) speed = 0;
    if (speed > 5.0) speed = 5.0; //Limitar la velocidad maxima a 5 m/s

    //Suponiendo que la velocidad maxima (5 m/s) corresponde al duty cycle maximo (255)
    float dutyCycle = (speed / 5.0) * 255.0;

    return (uint8_t)dutyCycle;
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

void Driver1::motor1_set_speed(float speed_ms){
    uint8_t pwm_value = motor_speed_to_dutyCycle(speed_ms);

    //Establecer el duty cycle para el motor 1
    ledc_set_duty(MOTOR_MODE, LEDC_CHANNEL_0, pwm_value);
    ledc_update_duty(MOTOR_MODE, LEDC_CHANNEL_0);

    motor1_speed = speed_ms;

    printf("Velocidad del motor 1 establecida a %.2f m/s (Duty Cycle: %d)\n", speed_ms, pwm_value);
}

void Driver1::motor2_set_speed(float speed_ms){
    uint8_t pwm_value = motor_speed_to_dutyCycle(speed_ms);

    //Establecer el duty cycle para el motor 2
    ledc_set_duty(MOTOR_MODE, LEDC_CHANNEL_1, pwm_value);
    ledc_update_duty(MOTOR_MODE, LEDC_CHANNEL_1);

    motor2_speed = speed_ms;

    printf("Velocidad del motor 2 establecida a %.2f m/s (Duty Cycle: %d)\n", speed_ms, pwm_value);
}
