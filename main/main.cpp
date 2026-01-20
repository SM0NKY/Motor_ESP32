#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include "GPS.h"

#define BLINKER_PIN GPIO_NUM_2

//Pines del GPS
#define GPS_TX_PIN GPIO_NUM_17
#define GPS_RX_PIN GPIO_NUM_16
#define GPS_UART_PORT UART_NUM_2

extern "C" void app_main(void) {

    GPS gps_UART(GPS_TX_PIN, GPS_RX_PIN, GPS_UART_PORT);

    gps_UART.init();

    //Setear el motor a on al inicio 

    while (1) {
        gps_UART.process_data();
        vTaskDelay(pdMS_TO_TICKS(10000));

        Data_GPS current_position = gps_UART.get_Data();
        if (current_position.isValid == true) {
            printf("Lat: %.6f | Lon: %.6f | Speed: %.2f | RAW_STATUS: ???\n",  
            current_position.latitude, 
            current_position.longitude,
            current_position.speed_kmh);
        }

        //Agregar un comando aqui para enviar el mensaje al topico correspondiente usando rcl
    }
}