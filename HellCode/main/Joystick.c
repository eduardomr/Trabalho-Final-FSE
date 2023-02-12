#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

#include "freeRTOS/semphr.h"
#include "freeRTOS/queue.h"
#include "wifi.h"
#include "mqtt.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"

#define JOYSTICK_X ADC1_CHANNEL_7
#define JOYSTICK_Y ADC1_CHANNEL_6

int posicaoX(){
 
  int posicaoX;

  posicaoX = adc1_get_raw(JOYSTICK_X);
  posicaoX = posicaoX - 512;


  return  posicaoX;
}

int posicaoY(){
 
  int posicaoY;

  posicaoY = adc1_get_raw(JOYSTICK_Y);
  posicaoY = posicaoY - 512;

  return posicaoY;
}