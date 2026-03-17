// Programa principal para los motores utilizando LORA -> ESP32 -> PWM
// Se utiliza CMAKE y se hace la build con esp-idf v5.5.3, flasheo con esptool.py

//Drivers//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#include "Joystic.h"
#include "Motor.h"
#include "Input_LoRa.h"

#define BLINKER_PIN GPIO_NUM_2

// --Pines de los motores -- //

//Motor derecho
#define FRONT_R_PWM GPIO_NUM_27
#define DIR_R GPIO_NUM_32

//Motor izquierdo
#define FRONT_L_PWM GPIO_NUM_26
#define DIR_L GPIO_NUM_25

// -- Pines del LoRa I2C -- //
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

extern "C" void app_main(void) 
{
    //Setear los parametros del I2C para el LoRa
    InputLoRa myLoRa(I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO, I2C_MASTER_NUM, I2C_MASTER_FREQ_HZ);
    Driver1 motors(FRONT_R_PWM, FRONT_L_PWM, DIR_R, DIR_L, 5.0); // Establecer velocidad máxima a 5 m/s

    

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

    }
}