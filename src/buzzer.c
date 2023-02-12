#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"


void liga_buzzer(int gpio){
   
    gpio_set_level(gpio, 1);
}

void desliga_buzzer(int gpio){
    
    gpio_set_level(gpio, 0);
}