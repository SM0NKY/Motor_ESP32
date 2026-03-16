#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <array>


using namespace std;

class Joystick_Converter{
    public:
        Joystick_Converter(int16_t X_position, int16_t Y_position); //Constructor con los valores de las posiciones del joystick

        array<float, 2> XY_Positions(); //Convierte la posición Y del joystick a una velocidad para

        void speed_ms_conversion(Driver1 &motor); //Convierte la posición del joystick a una velocidad para el motor

    private:
        
        float speed_ms;
        int16_t X, Y; //Posiciones del joystick en el eje X y Y

};