
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "freertos/event_groups.h"
// #include ""
// #include ""
// #include ""

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

#include "protocol_examples_common.h"

#include <esp_http_server.h>

////////////////////////////
#include "mdns.h" 

/*******************    Wifi ap config *******************/
#define MAX_CONNECTION 4
#define WIFI_CHANNEL  6
#define WIFI_BEACON_INTERVAL 100;

static esp_netif_t *esp_netif;

static EventGroupHandle_t wifi_events2;
static int CONNECTED =BIT0;
static int DISCONNECTED =BIT1;

char *get_wifi_disconnection_string(wifi_err_reason_t wifi_err_reason);

static const char *TAG = "WIFI CONNECT";

static const char *SECOND_TAG = "SERVER";

int conunt_failed_reconnection_times;
bool attempt_reconnect = false; 

static esp_err_t on_default_url(httpd_req_t *r)
{

    ESP_LOGI(SECOND_TAG,"URL: %s",r->uri);
    httpd_resp_sendstr(r,"hello world");
    return ESP_OK;

}

// Start the server
// handle uri
static void init_server()
{

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_uri_t default_url = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = on_default_url
  };
  httpd_register_uri_handler(server,&default_url);

}
static void send_task (void *params) 
{

cJSON *payload = cJSON_CreateObject();
cJSON_AddNumberToObject(payload,"CJ",7);
char *msg = cJSON_Print(payload);
printf("sending %s\n",msg);

cJSON_Delete(payload);
free(msg);


}

// To disconnecte the wifi. If needed

void wifi_disconnecte(void){
    attempt_reconnect = false;
    esp_wifi_stop();
    esp_netif_destroy(esp_netif);
}


void event_handler(void *event_handler_arg, esp_event_base_t event_base,
                   int32_t event_id, void *event_data)
{
    switch (event_id)
    {
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG,"WIFI_EVENT_AP_START");
            esp_wifi_connect();
            break;
        
        case WIFI_EVENT_AP_STOP:   
            ESP_LOGI(TAG,"WIFI_EVENT_AP_STOP");
            break;
        
        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG,"WIFI_EVENT_AP_STACONNECTED");
            conunt_failed_reconnection_times=0;
            break;
        
        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            //wifi_event_sta_disconnected_t *wifi_dis = event_data ; 

            wifi_event_ap_stadisconnected_t *wifi_disc = event_data ; 
            ESP_LOGI(TAG,"DISCONNECTED %d , %s", wifi_disc->mac[4],
            get_wifi_disconnection_string(wifi_disc->mac[4])
            );
            if(attempt_reconnect)
            {
                if (wifi_disc->mac[4] == 115){
                    ESP_LOGI(TAG, "Disonnected from Reguler");
                    // Try to reconnect 
                    if (conunt_failed_reconnection_times++ < 5)
                    {
                        ESP_LOGI("SA","S");
                        vTaskDelay(pdMS_TO_TICKS(2000));
                        esp_wifi_connect();
                        break;
                    }
                    //wifi_disconnecte();
                    

                }
            }
            //xEventGroupSetBits(wifi_events2,DISCONNECTED);

            break;
        }
        ////////////////////////////////// This event is in another file && it is for STA
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG,"IP_EVENT_STA_GOT_IP");
            //xEventGroupSetBits(wifi_events2,CONNECTED);
            break;

        default:
            break;
    }
    
}

// Enable Wifi for any mode 
void wifi_connect_init(void)
{


    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

}






// Configure AP and start it. 
void wifi_connect_ap(const char *ssid, const char *pass){

    /****************************** commented code is for STA ******************************
    wifi_events2 = xEventGroupCreate();

    ******************************************************************************************/
    attempt_reconnect = true;
    esp_netif = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char *)wifi_config.ap.password, pass, sizeof(wifi_config.ap.password) - 1);
    
    
    
    ////////////////Should we remove numbers and add variable?     
      
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.max_connection = 4; /// WIFI_MAX_CONNECTION;
    wifi_config.ap.beacon_interval = 100; //WIFI_BEACON_INTERVAL;
    wifi_config.ap.channel = 6; //WIFI_CHANNEL;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    /****************************** commented code is for STA ******************************
    EventBits_t result = (wifi_events2, CONNECTED | DISCONNECTED, true, false, pdMS_TO_TICKS(timeout) );
    if (result == CONNECTED) return ESP_OK;
    else return ESP_FAIL;

     ******************************************************************************************/

}

// We could remove it, if we will use Ip
void start_mdns_service()
{

    mdns_init();
    mdns_hostname_set("SI");
    mdns_instance_name_set("SC_SI");
}
void app_main(void){

    nvs_flash_init();
    wifi_connect_init();
    wifi_connect_ap("SSID","12345678");
    init_server();
    start_mdns_service();
    // esp_netif_init();
    // esp_event_loop_create_default();

    // example_connect();
    

}

