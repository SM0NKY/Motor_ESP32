#include "GPS.h"
#include <stio.h>
#include "stdlib.h"
#include <math.h>

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

void GPS::init(){
    uart_driver_install(GPS_UART_PORT_NUM, GPS_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(GPS_UART_PORT_NUM, &uart_config);

    uart_set_pin(GPS_UART_PORT_NUM, this->tx_pin, this->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    printf("--- GPS initialized on TX pin %d and RX pin %d\n", this->tx_pin, this->rx_pin);
}

float GPS::convert_nmea_to_decimal(float nmea_coord, chat quadrant){
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

    if (strncmp(sentence, "$GPRMC",6) == 0){
        //Dividir el texto en tokens
        char* token = strtok(sentence, ",");

        token = strtok(NULL, ","); //Tiempo Hora

        token = strtok(NULL, ","); //Estado A = Activo/Valido, V = Void/No valido

        if (token != NULL && strcmp(token, "A") == 0){
            this->data.is_valid = true;            
        } else {
            this->data.is_valid = false;
            return;
        }
        //Extraer datos

        //Latitud
        token = strtok(NULL, ",");
        float lat_raw = (token != NULL) ? atof(token): 0.0;

        //Hemisferio N/S
        token = strtok(NULL,",");
        char lat_dir = (token != NULL) ? token[0] : 'N';

        //Longitud
        token = strtok(NULL, ",");
        float lon_raw = (token != NULL) ? atof(token) : 0.0;

        //Hemisferio E/W
        token = strtok(NULL, ",");
        char lon_dir = (token != NULL) ? token[0] : 'E';

        //Velocidad en nudos
        token = strtok(NULL, ",");
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
}

void GPS::process_data(){
    uint_t temp_buffer[256];
    int lenght = uart_read_bytes(GPS_UART_PORT_NUM, temp_buffer, sizeof(temp_buffer) - 1, 100 / portTICK_PERIOD_MS);
   
    if (lenght > 0){
        temp_buffer[lenght] = '/0' ; //Null-terminate the string

        char* line = strtok ((char*)temp_buffer, "\n");
    }
}