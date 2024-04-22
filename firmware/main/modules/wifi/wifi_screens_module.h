#include "esp_wifi.h"
#ifndef WIFI_SCREENS_MODULE_H
  #define WIFI_SCREENS_MODULE_H
  #define TAG_WIFI_SCREENS_MODULE "module:wifi_screens"

void wifi_screens_module_scanning(void);
void wifi_screens_module_attacking(void);
void wifi_screens_module_display_scanned_networks(wifi_ap_record_t* ap_records,
                                                  int scanned_records,
                                                  int current_option);
void wifi_screens_module_display_details_network(wifi_ap_record_t* ap_record,
                                                 int page);
void wifi_screens_module_display_attack_selector(char* attack_options[],
                                                 int current_option);
#endif  // WIFI_SCREENS_MODULE_H
