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
    
    float max_motor_speed = 5.0f; // Velocidad máxima del motor en m/s
    Driver1 motors(FRONT_R_PWM, FRONT_L_PWM, DIR_R, DIR_L, max_motor_speed); // Establecer velocidad máxima a 5 m/s
    Joystick_Converter Joystick_Converter(max_motor_speed);

    //Setear el motor a on al inicio 

    while (1) {
        myLoRa.read_data(); // Leer datos del LoRa
        Joystick_Converter.speed_ms_conversion(myLoRa.x, myLoRa.y, myLoRa.brakes, [&motors]() { motors.motor1_direction_toggle(); }); // Convertir la posición del joystick a velocidad para el motor
        motors.motor1_set_speed(Joystick_Converter.speed_msy); // Establecer la velocidad del motor 1
        vTaskDelay(50 / portTICK_PERIOD_MS);

    }
}