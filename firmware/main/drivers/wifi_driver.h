#include "esp_wifi.h"
#ifndef WIFI_DRIVER_H
  #define WIFI_DRIVER_H
  #define TAG_WIFI_DRIVER                 "driver:wifi"
  #define WIFI_MANAGER_AP_SSID            CONFIG_MANAGER_AP_SSID
  #define WIFI_MANAGER_AP_PASSWORD        CONFIG_MANAGER_AP_PASSWORD
  #define WIFI_MANAGER_AP_MAX_CONNECTIONS CONFIG_MANAGER_AP_MAX_CONNECTIONS

wifi_config_t wifi_driver_access_point_begin(void);
void wifi_driver_ap_start(wifi_config_t* wifi_ap_config);
void wifi_driver_ap_stop(void);
void wifi_driver_init_apsta(void);
#endif  // WIFI_DRIVER_H
