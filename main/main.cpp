#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Blinker.h"
#include "Motor.h"

#define BLINKER_PIN GPIO_NUM_2

//Pines de los motores
#define PWM1 GPIO_NUM_23
#define PWM2 GPIO_NUM_22
#define DIR1 GPIO_NUM_1
#define DIR2 GPIO_NUM_3

// Esta línea es CRÍTICA
extern "C" void app_main(void) 
{
    Blinker myBlinker(BLINKER_PIN);
    Driver1 motors(PWM1, PWM2, DIR1, DIR2);

    //Setear el motor a on al inicio 

    while (1) {
        myBlinker.blink(1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        motors.motor1_set_speed(2.5);
        vTaskDelay(50/portTICK_PERIOD_MS);
        motors.motor1_set_speed(5.0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        motors.motor1_set_speed(0);
        
    }
}