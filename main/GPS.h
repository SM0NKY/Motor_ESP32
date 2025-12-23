#ifndef GPS_H
#define GPS_H

#include "driver/uart.h"
#include "driver/gpio.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define GPS_UART_PORT_NUM UART_NUM_2
#define GPS_BUF_SIZE 1024

//Estructura para almacenar los datos del GPS
struct Data_GPS {
    double latitude;
    double longitude;
    float speed_kmh;
    bool isValid;
};

class GPS{
    public:
        GPS(gpio_num_t tx_pin, gpio_num_t rx_pin, uart_port_t uart_num);

        void init();
        void process_data();

        Data_GPS get_Data();

    private:
        uart_port_t uart_num; 
        gpio_num_t tx_pin;
        gpio_num_t rx_pin;
        
        Data_GPS _data;

        //Usar mutex para evitar que se lean los datos cuando se esten cambiando 
        SemaphoreHandle_t _mutex; 

        void parse_nmea(char* sentence);
        double convert_nmea_to_decimal(float nmea_coord, char quadrant);

        char sentence_buf[256];
        int sentence_len;
};

#endif
