#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
//#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "dht11.h"

//#include "nvs_handler.h"

void app_main(void)
{
        dht11_run();

}