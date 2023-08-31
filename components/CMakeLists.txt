idf_component_register(
    SRCS 
    "wifi_conn.c" 
    "wifi_err.c"
    INCLUDE_DIRS 
    "."
    REQUIRES
    esp_netif
    esp_wifi
    log
                    
)
