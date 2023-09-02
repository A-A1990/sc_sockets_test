// #include <stdio.h>
// #include "esp_timer.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "driver/gpio.h"
// #include "freertos/timers.h"
// #include "driver/twai.h"

// #include <string.h>
// #include "driver/twai.h"
// #include "freertos/semphr.h"
// #include "esp_vfs_fat.h"
// #include "sdmmc_cmd.h"
// #include "driver/sdmmc_host.h"
// #include "sys/time.h" //

// #define CAN_TX_PIN 19
// #define CAN_RX_PIN 21



// int Co1 =0;
// int Co2 =0; 

// TaskHandle_t handleT1 = NULL;

// void task1(){

//     while (1)
//     {
//         ESP_LOGI("Task 1","T1: %d \n",Co1);
//         Co1++;
//         vTaskDelay(pdMS_TO_TICKS(2500));
//     }
//     vTaskDelete(NULL);
    

// }

// void timer_callback(void *param)
// {
//   ESP_LOGI("Timer Callback","(:");
// }

// void app_main(void)
// {


//     xTaskCreate(task1,"T1",2000,NULL,1,NULL);
//     const esp_timer_create_args_t my_timer_args = {
//         .callback = &timer_callback,
//         .name = "My Timer"};
// ////////////////////////////////////////////////////////                    
//     esp_timer_handle_t timer_handler;
//     ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
//     ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 1000000));

//     // xTaskCreate(canTX,"canTX",3000,NULL,1,NULL);
//     // xTaskCreate(canRX,"canRX",3000,NULL,1,NULL);
    


    
//     while (true)
//     {
//         //esp_timer_dump(stdout);
//         //esp_err_t esp_timer_restart(timer_handler, 1000000);
//         //vTaskDelay(pdMS_TO_TICKS(1000));

//     }
// }