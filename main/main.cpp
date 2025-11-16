#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Blinker.h"
#include "Motor.h"

#define BLINKER_PIN GPIO_NUM_2
#define MOTOR_PIN_ACT GPIO_NUM_23
// Esta línea es CRÍTICA
extern "C" void app_main(void) 
{
    Blinker myBlinker(BLINKER_PIN);
    Motor myMotor(MOTOR_PIN_ACT);

    while (1) {
        myBlinker.blink(1000);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        myMotor.toggle();
    }
}