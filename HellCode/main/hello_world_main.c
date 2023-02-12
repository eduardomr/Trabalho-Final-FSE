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
#define JOYSTICK_BOTAO 0
#define MASSIVE_LAZER 13

QueueHandle_t filaDeInterrupcoes;
SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;

void setup_gpio(){
  esp_rom_gpio_pad_select_gpio(JOYSTICK_BOTAO);
  gpio_set_direction(JOYSTICK_BOTAO, GPIO_MODE_INPUT);
  gpio_set_direction(MASSIVE_LAZER, GPIO_MODE_OUTPUT);

  // Configura o conversor AD
  adc1_config_width(ADC_WIDTH_BIT_10);
  adc1_config_channel_atten(JOYSTICK_X, ADC_ATTEN_DB_6);
  adc1_config_channel_atten(JOYSTICK_Y, ADC_ATTEN_DB_6);
  
}

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

void conectadoWifi(void * params)
{
  while(true)
  {
    if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
    {
      // Processamento Internet
      mqtt_start();
    }
  }
}

void trataComunicacaoComServidor(void * params)
{
  char JsonAtributos[200];
  if(xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
  {
    while(true)
    {
      int posX;
      int posY;
      posX = posicaoX();
      posY = posicaoY();
       
       sprintf(JsonAtributos, "{\"joyx\": %d}", posX);
       mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);

       sprintf(JsonAtributos, "{\"joy\": %d}", posY);
       mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);

       /* sprintf(JsonAtributos, "{\"quantidade de pinos\": 5,\n\"umidade\":  20}");
       mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos); */
       vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

static void IRAM_ATTR gpio_isr_handler(void* args)
{
  int pino = (int) args;
  xQueueSendFromISR(filaDeInterrupcoes, &pino, NULL);
}

void trataInterrupcaoBotao(void * params)
{
  int pino;
  char JsonAtributos[200];
  int estado = 0;
  printf("entrou no tratamento\n");

  while(true)
  {
   if(xQueueReceive(filaDeInterrupcoes, &pino, portMAX_DELAY))
   {
    
      if (estado == 0)
      {
        
        printf("LIGOU O LASER\n");
        gpio_set_level(MASSIVE_LAZER, 1);
        sprintf(JsonAtributos, "{\"estado_laser\": true}");
        mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);
        estado = 1;
      }
      else
      {
        printf("DESLIGOU O LASER\n");
        gpio_set_level(MASSIVE_LAZER, 0);
        sprintf(JsonAtributos, "{\"estado_laser\": false}");
        mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);
        estado = 0;
      }
    }
   }
  
}

void app_main()
{
  nvs_flash_init();
  setup_gpio();
  conexaoWifiSemaphore = xSemaphoreCreateBinary();
  conexaoMQTTSemaphore = xSemaphoreCreateBinary();
  filaDeInterrupcoes = xQueueCreate(10, sizeof(int));
  wifi_start();

  gpio_install_isr_service(0);
  gpio_isr_handler_add(JOYSTICK_BOTAO, gpio_isr_handler,(void*) JOYSTICK_BOTAO);

  xTaskCreate(conectadoWifi, "conectadoWifi", 2048, NULL, 5, NULL);
  xTaskCreate(trataInterrupcaoBotao, "trataInterrupcaoBotao", 2048, NULL, 5, NULL);
  xTaskCreate(&trataComunicacaoComServidor, "Comunicacao com Broker", 4096, NULL, 5, NULL);

}






