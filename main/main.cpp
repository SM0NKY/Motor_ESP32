#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

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
#define GPS_UART_PORT UART_NUM_2

//Tarea del segundo hilo
void gps_task(void *pvParameters){
    GPS* gps = (GPS*)pvParameters;
    while (1){
        gps->process_data();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

extern "C" void app_main(void) {
    Blinker myBlinker(BLINKER_PIN);
    Driver1 motors(PWM1, PWM2, DIR1, DIR2);

    GPS gps(GPS_TX_PIN, GPS_RX_PIN, GPS_UART_PORT);
    gps.init();
    xTaskCreatePinnedToCore(gps_task, "gps_task", 4096, &gps, 5, NULL, 1);
    printf("Inicializacion completa.\n");
    //Setear el motor a on al inicio 

    while (1) {
        myBlinker.blink(1000);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        Data_GPS current_ps = gps.get_Data();
        if (current_ps.isValid){
            printf("GPS Data - Latitude: %.6f, Longitude: %.6f, Speed: %.2f km/h\n",
                   current_ps.latitude,
                   current_ps.longitude,
                   current_ps.speed_kmh);
        } else {
            printf("GPS Data - No valid fix.\n");
        }
        
        motors.motor1_linear_increase(3.0, 5000); // Aumentar la velocidad del motor 1 a 2.5 m/s en 5 segundos
        vTaskDelay(pdMS_TO_TICKS(2000));
        motors.motor1_linear_increase(0.0, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motors.motor1_linear_increase(2.0, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motors.motor1_linear_increase(0.0, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        //Aqui se agrega el motor2 
        motors.motor2_linear_increase(1.0, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motors.motor2_linear_increase(1.2, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        gps.process_data();
    }
}