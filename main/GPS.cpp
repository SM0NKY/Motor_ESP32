#include "GPS.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


GPS::GPS(gpio_num_t tx_pin, gpio_num_t rx_pin, uart_port_t uart_num){
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
    this->uart_num = uart_num;
    this->sentence_len = 0;
    memset(this->sentence_buf, 0, sizeof(this->sentence_buf));

    //Crear mutex para proteger el acceso a los datos
    _mutex = xSemaphoreCreateMutex();

    _data.latitude = 0.0;
    _data.longitude = 0.0;
    _data.speed_kmh = 0.0;
    _data.isValid = false;
}

void GPS::init(){
    uart_config_t uart_config = {};
    uart_config.baud_rate = 9600;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

#if defined(UART_SCLK_DEFAULT)
    uart_config.source_clk = UART_SCLK_DEFAULT;
#else
    uart_config.source_clk = UART_SCLK_APB;
#endif

    uart_driver_install(uart_num, GPS_BUF_SIZE * 2, 0, 0, nullptr, 0);
    uart_param_config(uart_num, &uart_config);

    uart_set_pin(uart_num, this->tx_pin, this->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_flush_input(uart_num);

    printf("--- GPS initialized (NEO-6M) UART%d @ 9600 bps TX=%d RX=%d\n",
       (int)uart_num,
       (int)this->tx_pin,
       (int)this->rx_pin);
}

double GPS::convert_nmea_to_decimal(float nmea_coord, char quadrant){
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

        bool valid_now = (token != NULL && strcmp(token, "A") == 0);
        if (!valid_now){
            if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE){
            this->_data.isValid = false;
            xSemaphoreGive(_mutex);
            }
            return;
        }         
        //Extraer datos

        //Latitud
        token = strtok_r(nullptr, ",", &saveptr);
        float lat_raw = (token != NULL) ? atof(token): 0.0;

        token = strtok_r(nullptr, ",", &saveptr);
        char lat_dir = (token != NULL) ? token[0] : 'N';

        //Longitud
        token = strtok_r(nullptr, ",", &saveptr);
        float lon_raw = (token != NULL) ? atof(token) : 0.0;

        token = strtok_r(nullptr, ",", &saveptr);
        char lon_dir = (token != NULL) ? token[0] : 'E';

        //Velocidad en nudos
        token = strtok_r(nullptr, ",", &saveptr);
        float speed_knots = (token != NULL) ? atof(token) : 0.0;

        //Convertir y almacenar los datos
        if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE){
            this->_data.isValid = true;
            this->_data.latitude = convert_nmea_to_decimal(lat_raw, lat_dir);
            this->_data.longitude = convert_nmea_to_decimal(lon_raw, lon_dir);
            this->_data.speed_kmh = speed_knots * 1.852; //Conversion de nudos a km/h
            xSemaphoreGive(_mutex);
        }
        
    }

    else if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0){
        char* saveptr = nullptr;
        char* token = strtok_r(sentence, ",", &saveptr);

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

        token = strtok_r(nullptr, ",", &saveptr); // number of satellites
        token = strtok_r(nullptr, ",", &saveptr); // horizontal dilution of position

        token = strtok_r(nullptr, ",", &saveptr); // altitude (meters)
        // float altitude_m = (token != NULL) ? atof(token) : 0.0f;

        if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE){
            if (fix_quality > 0){
                this->_data.isValid = true;
                if (lat_raw != 0.0f) this->_data.latitude = convert_nmea_to_decimal(lat_raw, lat_dir);
                if (lon_raw != 0.0f) this->_data.longitude = convert_nmea_to_decimal(lon_raw, lon_dir);

            } else {
                this->_data.isValid = false;
            }
            xSemaphoreGive(_mutex);
        }
    }
}

void GPS::process_data(){
    uint8_t rx_buf[256];
    const int length = uart_read_bytes(uart_num, rx_buf, sizeof(rx_buf), pdMS_TO_TICKS(100));
    if (length <= 0) return;

    for (int i = 0; i < length; i++){
        const char c = (char)rx_buf[i];

        if (c == '\r') continue;

        if (c == '\n'){
            if (this->sentence_len > 0){
                this->sentence_buf[this->sentence_len] = '\0';

                printf("RAW NMEA: %s\n", this->sentence_buf); //Borrar luego esta linea

                parse_nmea(this->sentence_buf);
                this->sentence_len = 0;
            }
            continue;
        }

        if (c == '$'){
            this->sentence_len = 0;
        }

        if (this->sentence_len < (sizeof(this->sentence_buf) - 1)){
            this->sentence_buf[this->sentence_len++] = c;
        } else {
            this->sentence_len = 0;
        }
    }
}

Data_GPS GPS::get_Data(){
    Data_GPS dataCopy;
    
    dataCopy.latitude = 0.0;
    dataCopy.longitude = 0.0;
    dataCopy.speed_kmh = 0.0;
    dataCopy.isValid = false;

    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE){
        dataCopy = this->_data;
        xSemaphoreGive(_mutex);
    }

    return dataCopy;
}