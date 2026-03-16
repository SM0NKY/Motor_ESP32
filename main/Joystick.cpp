#include "Joystic.h"
#include "Motor.h"
#include <stdio.h>
#include <array>

Joystick_Converter::Joystick_Converter(int16_t X_position, int16_t Y_position){
    this->speed_ms = 0.0f;

    array<float, 2> XY_Positions(){
        //Convertir las posiciones
        float deadzone = 0.05f
        
        int32_t normX = ((int32_t)X_position - 32768) * 2; // Normalizar a un rango de -65536 a 65536
    }


};
