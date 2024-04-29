#include "trackers_scanner.h"
#ifndef BLE_MODULE_H
  #define BLE_MODULE_H
  #define TAG_BLE_MODULE "ble_module:main"

typedef struct {
  bool in_app;
  int app_selected;
} app_screen_state_information_t;

void ble_module_begin(int app_selected);
#endif  // BLE_MODULE_H
