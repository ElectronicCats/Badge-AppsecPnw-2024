#include "trackers_scanner.h"
#ifndef BLE_MODULE_H
  #define BLE_MODULE_H
  #define TAG_BLE_MODULE "ble_module:main"

/**
 * @brief Structure to store the app screen state information
 *
 */
typedef struct {
  bool in_app;
  int app_selected;
} app_screen_state_information_t;

/**
 * @brief Begin the bluetooth module
 *
 * @param app_selected The selected app
 */
void ble_module_begin(int app_selected);
#endif  // BLE_MODULE_H
