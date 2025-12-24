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

//Definir el constructor de la clase del GPS como variable global
GPS gps(GPS_TX_PIN, GPS_RX_PIN, GPS_UART_PORT);

//Tarea del segundo hilo
void gps_aktivität(void *pvParameters){
    GPS* gps = (GPS*)pvParameters;

    static double last_lat = 0.0;
    static double last_lon = 0.0;

    TickType_t last_wake_time = 0;

    while (1){
        gps->process_data();
    
        if ((xTaskGetTickCount() - last_wake_time) > pdMS_TO_TICKS(1000)){
            
            Data_GPS current_ps = gps->get_Data();

        if (current_ps.isValid == true && (current_ps.latitude != last_lat || current_ps.longitude != last_lon)){
            printf("Lat: %.6f | Lon: %.6f | Speed: %.2f | RAW_STATUS: ???\n",  
            current_ps.latitude, 
            current_ps.longitude,
            current_ps.speed_kmh);

            last_lat = current_ps.latitude;
            last_lon = current_ps.longitude;
        }
            last_wake_time = xTaskGetTickCount();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        
    }
}

extern "C" void app_main(void) {
    Blinker myBlinker(BLINKER_PIN);
    Driver1 motors(PWM1, PWM2, DIR1, DIR2);

    gps.init();
    xTaskCreatePinnedToCore(gps_aktivität, "gps_aktivität", 4096, &gps, 5, NULL, 1);
    printf("Inicializacion completa.\n");
    //Setear el motor a on al inicio 

    while (1) {

        myBlinker.blink(1000);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        
        myBlinker.blink(1000);
        vTaskDelay(pdMS_TO_TICKS(1000));

        motors.motor1_linear_increase(3.0, 5000); // Aumentar la velocidad del motor 1 a 2.5 m/s en 5 segundos
        vTaskDelay(pdMS_TO_TICKS(2000));
        motors.motor1_linear_increase(0.0, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        //Aqui se agrega el motor2 
        motors.motor2_linear_increase(1.0, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motors.motor2_linear_increase(1.2, 5000);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}