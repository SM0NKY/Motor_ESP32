#include "Joystic.h"
#include "Motor.h"
#include <stdio.h>
#include <array>

Joystick_Converter::Joystick_Converter(float max_speed_ms){
    MAX_SPEED_MS = max_speed_ms;

};

void Joystick_Converter::speed_ms_conversion(int16_t X_position, int16_t Y_position, int8_t brakes, std::function<void()> callback){ //Donde switch_dir es un puntero a función que se llamará para cambiar la dirección del motor si es necesario
    
    int32_t diffX = (int32_t)X_position - this->CENTRO;
    int32_t diffY = (int32_t)Y_position - this->CENTRO;

    if (abs(diffX) < RANGO_DEADZONE) diffX = 0; // Zona muerta para el eje X
    if (abs(diffY) < RANGO_DEADZONE) diffY = 0; // Zona muerta para el eje Y

    float normX = (float)diffX / 32768.0f; 
    float normY = (float)diffY / 32768.0f; 
    
    float expo_n = (1.0f - this->expo_factor)*normX + this->expo_factor*(normX*normX*normX); // Aplicar función exponencial para suavizar la respuesta
    float expo_y = (1.0f - this->expo_factor)*normY + this->expo_factor*(normY*normY*normY); // Aplicar función exponencial para suavizar la respuesta

    float baseSpeed = expo_y * this->MAX_SPEED_MS; // Escalar a la velocidad máxima
    
    this->speed_msy = baseSpeed; // Velocidad frontal basada en el eje Y

    //Posterior uso de dir aqui dir para setear la dirección del motor
    // if (expo_n >= 0){
    //     this->speed_msx = baseSpeed * (1.0f - expo_n); // Reducir velocidad lateral al girar a la derecha
    //     this->speed_msy = baseSpeed * (1.0f - expo_n); // Reducir velocidad frontal al girar a la derecha
    //     // Añadir un atributo linkeado de la clase del driver para elegir la dirección del motor
    
    // } else {
    //     this->speed_msy = baseSpeed * (1.0f + expo_n); // Reducir velocidad frontal al girar a la izquierda
    // }
}