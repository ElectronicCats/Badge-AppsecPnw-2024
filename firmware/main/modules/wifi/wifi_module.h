#include "esp_wifi.h"
#include "modules/cross_keyboard_module.h"
#include "modules/menu_screens_modules.h"

#ifndef WIFI_MODULE_H
  #define WIFI_MODULE_H
  #define TAG_WIFI_MODULE "module:wifi"

typedef enum {
  WIFI_STATE_SCANNING = 0,
  WIFI_STATE_SCANNED,
  WIFI_STATE_DETAILS,
  WIFI_STATE_ATTACK_SELECTOR,
  WIFI_STATE_ATTACK
} wifi_state_t;

typedef struct {
  wifi_state_t state;
  wifi_config_t wifi_config;
  wifi_ap_record_t ap_records[20];
  int total_scanned;
} wifi_module_t;

static char* wifi_attacks[] = {"Broadcast", "Rouge AP", "Combine"};

void wifi_module_begin(void);
void wifi_module_state_machine(button_event_t button_pressed);
#endif  // WIFI_MODULE_H
