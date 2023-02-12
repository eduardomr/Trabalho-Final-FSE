#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "buzzer.h"
#include "wifi.h"
#include "mqtt.h"
#include "encoderRotativo.h"

SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;
QueueHandle_t filaDeInterrupcoes;

#define BOTAO 0
#define LED_ESP 2
#define BUZZER 15
#define TEMP_CRIT 28.0

void setup_gpio()
{
    esp_rom_gpio_pad_select_gpio(LED_ESP);
    gpio_set_direction(LED_ESP, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(BOTAO);
    gpio_pulldown_en(BOTAO);
    gpio_pullup_dis(BOTAO);
    gpio_set_intr_type(BOTAO, GPIO_INTR_POSEDGE);
    gpio_set_direction(BOTAO, GPIO_MODE_INPUT);
    esp_rom_gpio_pad_select_gpio(BUZZER);
    gpio_set_direction(BUZZER, GPIO_MODE_OUTPUT);
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
  char mensagem[50];
  char JsonAtributos[200];
  if(xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
  {
    while(true)
    {
       float temperatura = 20.0 + (float)rand()/(float)(RAND_MAX/10.0);
       if (temperatura >= TEMP_CRIT)
       {
         // liga_buzzer(BUZZER);
        sprintf(JsonAtributos, "{\"crit_temp\": true}");
        mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);

       }
       else
       {
        // desliga_buzzer(BUZZER);
        sprintf(JsonAtributos, "{\"crit_temp\": false}");
        mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);
       }
       sprintf(mensagem, "{\"temperatura\": %f}", temperatura);
       mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);

       /* sprintf(JsonAtributos, "{\"quantidade de pinos\": 5,\n\"umidade\":  20}");
       mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos); */
       vTaskDelay(3000 / portTICK_PERIOD_MS);
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


  while(true)
  {
   if(xQueueReceive(filaDeInterrupcoes, &pino, portMAX_DELAY))
   {
    
      if (estado == 0)
      {
        
        printf("LIGOU O SISTEMA\n");
        gpio_set_level(LED_ESP, 1);
        sprintf(JsonAtributos, "{\"estado_sistema\": true}");
        mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);
        estado = 1;
      }
      else
      {
        printf("DESLIGOU O SISTEMA\n");
        gpio_set_level(LED_ESP, 0);
        sprintf(JsonAtributos, "{\"estado_sistema\": false}");
        mqtt_envia_mensagem("v1/devices/me/attributes", JsonAtributos);
        estado = 0;
      }
    }
   }
  
}
void app_main(void)
{
  

  nvs_flash_init();
  setup_gpio();
  
  conexaoWifiSemaphore = xSemaphoreCreateBinary();
  conexaoMQTTSemaphore = xSemaphoreCreateBinary();
  filaDeInterrupcoes = xQueueCreate(10, sizeof(int));
  wifi_start();
  
  
  xTaskCreate(conectadoWifi, "conectadoWifi", 2048, NULL, 5, NULL);
  xTaskCreate(trataInterrupcaoBotao, "trataInterrupcaoBotao", 2048, NULL, 5, NULL);
  xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(BOTAO, gpio_isr_handler,(void*) BOTAO);
}