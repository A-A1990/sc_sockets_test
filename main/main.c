
#include <stdio.h>


#include "freertos/FreeRTOS.h"

#include "freertos/task.h"

#include "freertos/semphr.h"



#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include <esp_http_server.h>
#include "esp_timer.h"

#include "sys/socket.h"

// #include "esp_eth.h"


// #include "protocol_examples_common.h"




////////////////////////////
// #include "mdns.h" 

//#include "toggleled.h"

/*******************    Wifi ap config *******************/

/***********************************    Importent **************************/
/*******************    Before use We need to uncomment the usege of this in wifi_connect_ap()*******************/
#define MAX_CONNECTION 4
#define WIFI_CHANNEL  6
#define WIFI_BEACON_INTERVAL 100;

/*******************    Wifi ap config *******************/


#define LED 37
#define BTN 11

#define seconds_to_micro 1000000

static SemaphoreHandle_t btn_sem;

int websocket_counter = 0;
bool websocket_closed = false;


static esp_netif_t *esp_netif;

// static EventGroupHandle_t wifi_events2;
// static int CONNECTED =BIT0;
// static int DISCONNECTED =BIT1;

char *get_wifi_disconnection_string(wifi_err_reason_t wifi_err_reason);

static const char *TAG = "WIFI CONNECT";

static const char *SECOND_TAG = "SERVER";
static const char *on_ws_tag = "on_ws_url";

// int conunt_failed_reconnection_times;
// bool attempt_reconnect = false; 

static httpd_handle_t server = NULL;
esp_timer_handle_t timer_handler;

/*static void IRAM_ATTR on_btn_pushed(void *args)
{
  xSemaphoreGiveFromISR(btn_sem, NULL);
}

static void btn_push_task(void *params)
{
  while (true)
  {
    xSemaphoreTake(btn_sem, portMAX_DELAY);
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddBoolToObject(payload, "btn_state", gpio_get_level(BTN));
    char *message = cJSON_Print(payload);
    ESP_LOGW("btn","message: %s\n", message);
    //send_ws_message(message);
    cJSON_Delete(payload);
    free(message);
  }
}
void init_btn(void)
{
  xTaskCreate(btn_push_task, "btn_push_task", 2048, NULL, 5, NULL);
  btn_sem = xSemaphoreCreateBinary();
  // not required for version 5
  // gpio_pad_select_gpio(BTN);
  gpio_set_direction(BTN, GPIO_MODE_INPUT);
  gpio_set_intr_type(BTN, GPIO_INTR_ANYEDGE);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(BTN, on_btn_pushed, NULL);
}
*/

void pack(){
    // uint8_t buf[6];
    // int bufLength = sizeof(buf);
    // char esp_id[2 * bufLength + 1];
    // // Convert uint8_t buffer to string
    // for (int i = 0; i < bufLength; i++)
    // {
    //     sprintf(&esp_id[i * 2], "%02X", buf[i]); // Format as two-digit hexadecimal
    // }

    // char *timestamp = esp_log_system_timestamp();

}

void inint_led(void){ 

    //esp_rom_gpio_pad_select_gpio(LED);
    gpio_set_direction(LED,GPIO_MODE_OUTPUT);
    
}
void toggle_led(bool is_on){

    gpio_set_level(LED,is_on);
}

static esp_err_t on_default_url(httpd_req_t *r)
{
    ESP_LOGI(SECOND_TAG,"URL: %s",r->uri);
    httpd_resp_sendstr(r,"hello world again");
    return ESP_OK;
}

static esp_err_t on_toggle_led_url (httpd_req_t *r)
{
    char buffer[100];
    memset(&buffer,0,sizeof(buffer));
    ESP_LOGI("Led","URL: %s",r->uri);

    if(r->content_len<sizeof(buffer)){
        httpd_req_recv(r,buffer,r->content_len);
         ///////////////////////////////////     We need to check if there is an error. Both cJson should not return null     //////////////// 
        cJSON *paloayd= cJSON_Parse(buffer);
        cJSON *is_on_Json= cJSON_GetObjectItem(paloayd,"is_on");

        bool is_on = cJSON_IsTrue(is_on_Json);
        
        cJSON_Delete(paloayd);
        ESP_LOGW("Handler","is_on: %d",is_on);
        toggle_led(is_on);

        httpd_resp_set_status(r,"204 NO CONTENT");
        // We don't want to send anything
        httpd_resp_send(r,NULL,0);

        
        
    }
    return ESP_OK;
    
                                                                                           
}

////////////////////////////// Move it up 
static int client_session_id; 
#define WS_MAX_SIZE 1024


static esp_err_t on_ws_url(httpd_req_t *r)
{
    ESP_LOGI("ES","URL: %s",r->uri);
    client_session_id = httpd_req_to_sockfd(r);
    websocket_closed = false;
    if(r->method == HTTP_GET){
        return ESP_OK;
    }
    
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt,0,sizeof(httpd_ws_frame_t()));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = malloc(WS_MAX_SIZE);
    httpd_ws_recv_frame(r,&ws_pkt,WS_MAX_SIZE); // if last parmeter 0 , then we will got the length, but it give error

    ESP_LOGI("WebSocket","WS payload%.*s\n", ws_pkt.len,ws_pkt.payload);
    printf("Web_socket: %.*s\n", ws_pkt.len,ws_pkt.payload);
    free(ws_pkt.payload);

    char *response = "connected ok";

    httpd_ws_frame_t ws_respon3 = {
        .final=true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)response,
        .len = strlen(response)
    };
    
    return httpd_ws_send_frame(r,&ws_respon3);
    
}

esp_err_t send_ws_ms(char* msg){
    //ESP_LOGE(on_ws_tag,"In start of send_ws_ms");
    httpd_ws_frame_t ws_msg = {

        .final = true,
        .fragmented =false,
        .len = strlen(msg),
        .payload= (uint8_t *) msg,
        .type = HTTPD_WS_TYPE_TEXT
    };
    
    if(!client_session_id)
    {
        ESP_LOGE(on_ws_tag, "Ther is no client id");
        return -1;
    }
    int sending_frame_status = httpd_ws_send_frame_async(server,client_session_id,&ws_msg);
    if (sending_frame_status==0)
    {        
        ESP_LOGI(on_ws_tag, "Client id %d",client_session_id);
        
        websocket_counter = 0;
        websocket_closed = false;
        return sending_frame_status;
    }
    else if(websocket_counter >= 10 && sending_frame_status !=0 && websocket_closed == false )
    {

        int ws_trigger_status = httpd_sess_trigger_close(server, client_session_id);
        websocket_closed = true;
        ESP_LOGI(on_ws_tag,"Socket close return: %d \n",ws_trigger_status);
        return -1;
    }
    
    else
    {
        websocket_counter++;
        // ESP_LOGI(on_ws_tag,"Counter: %d",websocket_counter);
        return -1;
    }

    


}
// Start the server
// handle uri
static void init_server()
{

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_uri_t default_url = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = on_default_url,
        .user_ctx = NULL
  };
  
    httpd_uri_t toggle_led_url = {
            .uri = "/led",
            .method = HTTP_POST,
            .handler = on_toggle_led_url,
            .user_ctx = NULL
    };

    httpd_uri_t ws_url = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = on_ws_url,
        .is_websocket = true,
        .user_ctx = NULL
  };

    // for http server
    httpd_register_uri_handler(server,&default_url);

    //for led
    httpd_register_uri_handler(server,&toggle_led_url);

    // for websocket
   httpd_register_uri_handler(server,&ws_url);

}

/*static void send_task (void *params) 
{

cJSON *payload = cJSON_CreateObject();
cJSON_AddNumberToObject(payload,"CJ",7);
char *msg = cJSON_Print(payload);
printf("sending %s\n",msg);

cJSON_Delete(payload);
free(msg);


}*/


// To disconnecte the wifi. If needed

void wifi_disconnecte(void){
    //attempt_reconnect = false;
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
            //conunt_failed_reconnection_times=0;
            break;
        
        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            //wifi_event_sta_disconnected_t *wifi_dis = event_data ; 

            wifi_event_ap_stadisconnected_t *wifi_disc = event_data ; 
            ESP_LOGI(TAG,"DISCONNECTED %d , %s", wifi_disc->mac[4],
            get_wifi_disconnection_string(wifi_disc->mac[4])
            );
            // if(attempt_reconnect)
            // {
            //     if (wifi_disc->mac[4] == 115){
            //         ESP_LOGI(TAG, "Disonnected from Reguler");
            //         // Try to reconnect 
            //         if (conunt_failed_reconnection_times++ < 5)
            //         {
            //             ESP_LOGI("SA","S");
            //             vTaskDelay(pdMS_TO_TICKS(2000));
            //             esp_wifi_connect();
            //             break;
            //         }
            //         //wifi_disconnecte();
                    

            //     }
            // }
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
    //attempt_reconnect = true;
    esp_netif = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char *)wifi_config.ap.password, pass, sizeof(wifi_config.ap.password) - 1);
    
          
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.max_connection = MAX_CONNECTION;
    wifi_config.ap.beacon_interval = WIFI_BEACON_INTERVAL;
    wifi_config.ap.channel = WIFI_CHANNEL;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    /****************************** commented code is for STA ******************************
    EventBits_t result = (wifi_events2, CONNECTED | DISCONNECTED, true, false, pdMS_TO_TICKS(timeout) );
    if (result == CONNECTED) return ESP_OK;
    else return ESP_FAIL;

     ******************************************************************************************/

}

// We could remove it, if we will use Ip
// void start_mdns_service()
// {
//     mdns_init();
//     mdns_hostname_set("si.local");
//     mdns_instance_name_set("SC_SI");
// }

void timer_callback2(){
    //ESP_LOGI("timer_callback","timer callback");
    if (!websocket_closed)
    {
        char *timestamp = esp_log_system_timestamp();
        // Create cJSON object/struct to store data
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "Timestamp", timestamp);
        char *msg = cJSON_Print(root);
        send_ws_ms(msg);

        cJSON_Delete(root);
        free(msg);
    }
    
}

void start_timer(){
ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, seconds_to_micro * 1.5 ));
}

void init_timer(){

    const esp_timer_create_args_t my_timer_args = {.callback = &timer_callback2, .name = "WebsocketTimer"};
    
    ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));

    start_timer();
    
}


void app_main(void){

    nvs_flash_init();
    inint_led();
    //init_btn();
    // start_mdns_service();
    wifi_connect_init();
    wifi_connect_ap("SSID","12345678");
    init_timer();
    
    init_server();
    
    // esp_netif_init();
    // esp_event_loop_create_default();

    // example_connect();
    

}

