#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void setup_encoder_rotativo(int botao, int clk, int dt) {
    esp_rom_gpio_pad_select_gpio(botao);
    gpio_pulldown_en(botao);
    gpio_pullup_dis(botao);
    gpio_set_intr_type(botao, GPIO_INTR_POSEDGE);
    gpio_set_direction(botao, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(clk);
    gpio_set_direction(clk, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(dt);
    gpio_set_direction(dt, GPIO_MODE_INPUT);
}

int valor_encoder_rotativo(int clk, int dt) {
    if(gpio_get_level(clk)){
            return 1;
        } 
         else if (gpio_get_level(dt)){
        return -1;
        }
    
    else {return 0;}
    

}