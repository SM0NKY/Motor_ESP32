#include "Blinker.h"
#include "freertos/task.h"

Blinker::Blinker(gpio_num_t numpin){
    pin = numpin;
    state = false;

    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

void Blinker::blink(int intervals_ms){
    state = !state;
    gpio_set_level(pin, state);
    vTaskDelay(intervals_ms / portTICK_PERIOD_MS);
}