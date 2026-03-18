#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <array>
#include <functional>


using namespace std;

class Joystick_Converter{
    public:
        Joystick_Converter(float max_speed_ms); //Constructor con los valores de las posiciones del joystick

        void speed_ms_conversion(int16_t X_position, int16_t Y_position, int8_t brakes, std::function<void()> callback); //Convierte la posición del joystick a una velocidad para el motor

        float speed_msy= 0.0f, speed_msx = 0.0f;
    private:
        float expo_factor = 0.7f; //Velocidad en m/s que se le asignará al motor
        float MAX_SPEED_MS; //Velocidad máxima del motor en m/s, se asignará en el constructor dependiendo del motor que se use
        int16_t RANGO_DEADZONE = 1500, CENTRO = 32768; //Rango de la zona muerta y el valor central del joystick
        
        

};

#endif