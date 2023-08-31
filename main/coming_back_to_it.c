
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include <esp_wifi.h>
// #include <esp_event.h>
// #include <esp_log.h>
// #include <esp_system.h>

// #include "nvs_flash.h"
// #include "esp_netif.h"
// #include "esp_wifi.h"
// #include "esp_event.h"

// #include "esp_eth.h"

// #include "cJSON.h"

// #include "protocol_examples_common.h"


// #include <esp_http_server.h>



// static const char *TAG = "SERVER";


// static esp_err_t on_default_url(httpd_req_t *r)
// {

//     ESP_LOGI(TAG,"URL: %s",r->uri);
//     httpd_resp_sendstr(r,"hello world");
//     return ESP_OK;

// }



// // Start the server
// // handle uri
// static void init_server()
// {

//     httpd_handle_t server = NULL;
//     httpd_config_t config = HTTPD_DEFAULT_CONFIG();

//     ESP_ERROR_CHECK(httpd_start(&server, &config));

//     httpd_uri_t default_url = {

//         .uri = "/",
//         .method = HTTP_GET,
//         .handler = on_default_url
//   };
//   httpd_register_uri_handler(server,&default_url);

// }
// static void send_task (void *params) 
// {

// cJSON *payload = cJSON_CreateObject();
// cJSON_AddNumberToObject(payload,"CJ",7);
// char *msg = cJSON_Print(payload);
// printf("sending %s\n",msg);

// cJSON_Delete(payload);
// free(msg);


// }

// void app_main(void){

//     nvs_flash_init();
//     esp_netif_init();
//     esp_event_loop_create_default();

//     example_connect();
    

// }

