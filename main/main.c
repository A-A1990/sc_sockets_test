
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "esp_eth.h"

#include "cJSON.h"


#include <esp_http_server.h>

static const char *TAG = "SERVER"

static void send_task (void *params) 
{

cJSON *payload = cJSON_CreateObject();
cJSON_AddNumberToObject(payload,"CJ",7);
char *msg = cJSON_Print(payload);
printf("sending %s\n",msg);

cJSON_Delete(payload);
free(msg);


}

void app_main(void){

    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    

}

