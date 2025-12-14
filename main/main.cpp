#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Blinker.h"
#include "Motor.h"
#include "GPS.h"

#define BLINKER_PIN GPIO_NUM_2

//Pines de los motores
#define PWM1 GPIO_NUM_23
#define PWM2 GPIO_NUM_22
#define DIR1 GPIO_NUM_18
#define DIR2 GPIO_NUM_19

//Pines del GPS
#define GPS_TX_PIN GPIO_NUM_17
#define GPS_RX_PIN GPIO_NUM_16

// Esta línea es CRÍTICA
extern "C" void app_main(void) 
{
    Blinker myBlinker(BLINKER_PIN);
    Driver1 motors(PWM1, PWM2, DIR1, DIR2);

    GPS gps(GPS_TX_PIN, GPS_RX_PIN);
    gps.init();
    //Setear el motor a on al inicio 

    while (1) {
        myBlinker.blink(1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        motors.motor1_linear_increase(3.0, 5000); // Aumentar la velocidad del motor 1 a 2.5 m/s en 5 segundos
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        motors.motor1_linear_increase(0.0, 5000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        motors.motor1_linear_increase(2.0, 5000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        motors.motor1_linear_increase(0.0, 5000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        //Aqui se agrega el motor2 
        motors.motor2_linear_increase(1,1, 5000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        motors.motor2_linear_increase(1.2, 5000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);


    }
}