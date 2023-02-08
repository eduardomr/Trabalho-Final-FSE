#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"


void liga_buzzer(int gpio){
    esp_rom_gpio_pad_select_gpio(gpio);
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, 1);
}

void desliga_buzzer(int gpio){
    esp_rom_gpio_pad_select_gpio(gpio);
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, 0);
}