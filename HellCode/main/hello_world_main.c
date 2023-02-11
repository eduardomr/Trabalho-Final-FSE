#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#define JOYSTICK_X ADC1_CHANNEL_7
#define JOYSTICK_Y ADC1_CHANNEL_6
#define JOYSTICK_BOTAO 33

void app_main()
{
  esp_rom_gpio_pad_select_gpio(JOYSTICK_BOTAO);
  gpio_set_direction(JOYSTICK_BOTAO, GPIO_PULLUP_PULLDOWN);
  gpio_pulldown_en(JOYSTICK_BOTAO);
  gpio_pullup_dis(JOYSTICK_BOTAO);

  // Configura o conversor AD
  adc1_config_width(ADC_WIDTH_BIT_10);
  adc1_config_channel_atten(JOYSTICK_X, ADC_ATTEN_DB_6);
  adc1_config_channel_atten(JOYSTICK_Y, ADC_ATTEN_DB_6);
  
  while (true)
  {
    int posicao_x = adc1_get_raw(JOYSTICK_X);
    int posicao_y = adc1_get_raw(JOYSTICK_Y);
    int botao = gpio_get_level(JOYSTICK_BOTAO);

    posicao_x = posicao_x - 512;
    posicao_y = posicao_y - 512;
    printf("Posição X: %.3d \t Posição Y: %.3d | Botão: %d\n", posicao_x, posicao_y, botao);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

}
