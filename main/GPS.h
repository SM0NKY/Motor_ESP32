#ifndef GPS_H
#define GPS_H

#include "driver/uart.h"
#include "driver/gpio.h"
#include <stddef.h>

#define GPS_UART_PORT_NUM UART_NUM_2
#define GPS_BUF_SIZE 1024

//Estructura para almacenar los datos del GPS
struct Data_GPS {
    float latitude;
    float longitude;
    float speed_kmh;
    float speed_ms;
    float altitude;
    bool is_valid;
};

class GPS {
    public:
        GPS(gpio_num_t tx_pin, gpio_num_t rx_pin);

        void init(int baud_rate = 9600);
        void process_data();

        Data_GPS data;

    private:
        gpio_num_t tx_pin;
        gpio_num_t rx_pin;
        int baud_rate = 9600;

        char sentence_buf[256] = {0};
        size_t sentence_len = 0;

        void parse_nmea(char* sentence);
        float convert_nmea_to_decimal(float nmea_coord, char quadrant);
};

#endif
