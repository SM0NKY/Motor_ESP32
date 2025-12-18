#include "GPS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

GPS::GPS(gpio_num_t tx_pin, gpio_num_t rx_pin){
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;

    data.latitude = 0.0;
    data.longitude = 0.0;
    data.speed_kmh = 0.0;
    data.speed_ms = 0.0;
    data.altitude = 0.0;
    data.is_valid = false;
}

void GPS::init(int baud_rate){
    this->baud_rate = baud_rate;

    uart_config_t uart_config = {};
    uart_config.baud_rate = this->baud_rate;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

#if defined(UART_SCLK_DEFAULT)
    uart_config.source_clk = UART_SCLK_DEFAULT;
#else
    uart_config.source_clk = UART_SCLK_APB;
#endif

    uart_driver_install(GPS_UART_PORT_NUM, GPS_BUF_SIZE * 2, 0, 0, nullptr, 0);
    uart_param_config(GPS_UART_PORT_NUM, &uart_config);

    uart_set_pin(GPS_UART_PORT_NUM, this->tx_pin, this->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_flush_input(GPS_UART_PORT_NUM);

    printf("--- GPS initialized (NEO-6M) UART%d @ %d bps TX=%d RX=%d\n",
           (int)GPS_UART_PORT_NUM,
           this->baud_rate,
           (int)this->tx_pin,
           (int)this->rx_pin);
}

float GPS::convert_nmea_to_decimal(float nmea_coord, char quadrant){
    //Separar los grados
    int degrees = (int)(nmea_coord / 100);

    //Separar los minutos
    float minutes = nmea_coord - (degrees * 100);

    //Conversion de unidades
    float decimal_coord = degrees + (minutes / 60.0);

    //Ajustar segun el cuadrante
    if(quadrant == 'S' || quadrant == 'W'){
        decimal_coord *= -1;
    }
    return decimal_coord;
}

void GPS::parse_nmea(char* sentence){

    if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0){
        //Dividir el texto en tokens
        char* saveptr = nullptr;
        char* token = strtok_r(sentence, ",", &saveptr);

        token = strtok_r(nullptr, ",", &saveptr); //Tiempo Hora

        token = strtok_r(nullptr, ",", &saveptr); //Estado A = Activo/Valido, V = Void/No valido

        if (token != NULL && strcmp(token, "A") == 0){
            this->data.is_valid = true;            
        } else {
            this->data.is_valid = false;
            return;
        }
        //Extraer datos

        //Latitud
        token = strtok_r(nullptr, ",", &saveptr);
        float lat_raw = (token != NULL) ? atof(token): 0.0;

        //Hemisferio N/S
        token = strtok_r(nullptr, ",", &saveptr);
        char lat_dir = (token != NULL) ? token[0] : 'N';

        //Longitud
        token = strtok_r(nullptr, ",", &saveptr);
        float lon_raw = (token != NULL) ? atof(token) : 0.0;

        //Hemisferio E/W
        token = strtok_r(nullptr, ",", &saveptr);
        char lon_dir = (token != NULL) ? token[0] : 'E';

        //Velocidad en nudos
        token = strtok_r(nullptr, ",", &saveptr);
        float speed_knots = (token != NULL) ? atof(token) : 0.0;

        //Convertir y almacenar los datos
        this->data.latitude = convert_nmea_to_decimal(lat_raw, lat_dir);
        this->data.longitude = convert_nmea_to_decimal(lon_raw, lon_dir);

        this->data.speed_kmh = speed_knots * 1.852; //Conversion de nudos a km/h
        this->data.speed_ms = this->data.speed_kmh / 3.6; //Conversion de km/h a m/s

        printf("GPS Data - Lat: %.6f, Lon: %.6f, Speed: %.2f km/h (%.2f m/s)\n",
               this->data.latitude,
               this->data.longitude,
               this->data.speed_kmh,
               this->data.speed_ms);
    }

    if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0){
        char* saveptr = nullptr;
        char* token = strtok_r(sentence, ",", &saveptr);
        (void)token;

        token = strtok_r(nullptr, ",", &saveptr); // time

        token = strtok_r(nullptr, ",", &saveptr); // lat
        float lat_raw = (token != NULL) ? atof(token) : 0.0f;
        token = strtok_r(nullptr, ",", &saveptr); // N/S
        char lat_dir = (token != NULL) ? token[0] : 'N';

        token = strtok_r(nullptr, ",", &saveptr); // lon
        float lon_raw = (token != NULL) ? atof(token) : 0.0f;
        token = strtok_r(nullptr, ",", &saveptr); // E/W
        char lon_dir = (token != NULL) ? token[0] : 'E';

        token = strtok_r(nullptr, ",", &saveptr); // fix quality
        int fix_quality = (token != NULL) ? atoi(token) : 0;
        if (fix_quality <= 0){
            this->data.is_valid = false;
            return;
        }

        token = strtok_r(nullptr, ",", &saveptr); // satellites
        token = strtok_r(nullptr, ",", &saveptr); // hdop

        token = strtok_r(nullptr, ",", &saveptr); // altitude (meters)
        float altitude_m = (token != NULL) ? atof(token) : 0.0f;

        this->data.is_valid = true;
        if (lat_raw != 0.0f) this->data.latitude = convert_nmea_to_decimal(lat_raw, lat_dir);
        if (lon_raw != 0.0f) this->data.longitude = convert_nmea_to_decimal(lon_raw, lon_dir);
        this->data.altitude = altitude_m;
    }
}

void GPS::process_data(){
    uint8_t rx_buf[256];
    const int length = uart_read_bytes(GPS_UART_PORT_NUM, rx_buf, sizeof(rx_buf), pdMS_TO_TICKS(100));
    if (length <= 0) return;

    for (int i = 0; i < length; i++){
        const char c = (char)rx_buf[i];

        if (c == '\r') continue;

        if (c == '\n'){
            if (sentence_len > 0){
                sentence_buf[sentence_len] = '\0';
                parse_nmea(sentence_buf);
                sentence_len = 0;
            }
            continue;
        }

        if (c == '$'){
            sentence_len = 0;
        }

        if (sentence_len < (sizeof(sentence_buf) - 1)){
            sentence_buf[sentence_len++] = c;
        } else {
            sentence_len = 0;
        }
    }
}
