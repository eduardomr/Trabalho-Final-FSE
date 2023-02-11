#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "dht11.h"

#include "wifi.h"
#include "mqtt.h"

#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#include "dht11.h"

#include "nvs_handler.h"

//temperature function start
static gpio_num_t dht_gpio = GPIO_NUM_4;
static int64_t last_read_time = -2000000;
static struct dht11_reading last_read;

float temperatura = 0.0;
float umidade = 0.0;

extern void mqtt_send_message(char *topic, char *menssage);

static int _waitOrTimeout(uint16_t microSeconds, int level)
{
    int micros_ticks = 0;
    while (gpio_get_level(dht_gpio) == level)
    {
        if (micros_ticks++ > microSeconds)
            return DHT11_TIMEOUT_ERROR;
        ets_delay_us(1);
    }
    return micros_ticks;
}

static int _checkCRC(uint8_t data[])
{
    if (data[4] == (data[0] + data[1] + data[2] + data[3]))
        return DHT11_OK;
    else
        return DHT11_CRC_ERROR;
}

static void _sendStartSignal()
{
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    ets_delay_us(20 * 1000);
    gpio_set_level(dht_gpio, 1);
    ets_delay_us(40);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);
}

static int _checkResponse()
{

    if (_waitOrTimeout(80, 0) == DHT11_TIMEOUT_ERROR)
        return DHT11_TIMEOUT_ERROR;

    
    if (_waitOrTimeout(80, 1) == DHT11_TIMEOUT_ERROR)
        return DHT11_TIMEOUT_ERROR;

    return DHT11_OK;
}

static struct dht11_reading _timeoutError()
{
    struct dht11_reading timeoutError = {DHT11_TIMEOUT_ERROR, -1, -1};
    return timeoutError;
}

static struct dht11_reading _crcError()
{
    struct dht11_reading crcError = {DHT11_CRC_ERROR, -1, -1};
    return crcError;
}

struct dht11_reading DHT11_read()
{
    if (esp_timer_get_time() - 2000000 < last_read_time)
    {
        return last_read;
    }

    last_read_time = esp_timer_get_time();

    uint8_t data[5] = {0, 0, 0, 0, 0};

    _sendStartSignal();

    if (_checkResponse() == DHT11_TIMEOUT_ERROR)
        return last_read = _timeoutError();

    for (int i = 0; i < 40; i++)
    {
        
        if (_waitOrTimeout(50, 0) == DHT11_TIMEOUT_ERROR)
            return last_read = _timeoutError();

        if (_waitOrTimeout(70, 1) > 28)
        {
            
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    if (_checkCRC(data) != DHT11_CRC_ERROR)
    {
        last_read.status = DHT11_OK;
        last_read.temperature = data[2];
        last_read.humidity = data[0];
        return last_read;
    }
    else
    {
        return last_read = _crcError();
    }
}
//end of temperature function

SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;

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
       sprintf(mensagem, "{\"temperatura\": %f}", temperatura);
       mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);

       sprintf(JsonAtributos, "{\"quantidade de pinos\": 5,\n\"umidade\":  20}");
       mqtt_envia_mensagem("v1/devices/me/telemetry", JsonAtributos);
       vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
  }
}



void app_main(void)
{
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    xTaskCreate(&conectadoWifi,  "Conexão ao MQTT", 4096, NULL, 1, NULL);
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
    
    while (true)
    {
        dht11_run()
    }
}
