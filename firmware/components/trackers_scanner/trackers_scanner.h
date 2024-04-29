#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#ifndef TRACKERS_SCANNER_H
  #define TRACKERS_SCANNER_H
  #define TAG_BLE_CLIENT_MODULE           "trackers_module:main"
  #define TRACKER_REMOTE_SERVICE_UUID     0x00FF
  #define TRACKER_REMOTE_NOTIFY_CHAR_UUID 0xFF01
  #define TRACKER_PROFILE_NUM             1
  #define TRACKER_PROFILE_A_APP_ID        0
  #define TRACKER_INVALID_HANDLE          0
  #define TRACKER_SCAN_DURATION           60

typedef struct {
  int rssi;
  char* name;
  char* vendor;
  uint8_t mac_address[6];
  uint8_t adv_data[31];
  uint8_t adv_data_length;
  bool is_tracker;
} tracker_profile_t;

typedef struct {
  uint8_t adv_cmp[4];
  char* name;
  char* vendor;
} tracker_adv_cmp_t;

typedef void (*bluetooth_scanner_cb_t)(tracker_profile_t record);

void trackers_scanner_init();
void trackers_scanner_register_cb(bluetooth_scanner_cb_t cb);
void trackers_scanner_start();
void trackers_scanner_stop();
bool trackers_scanner_is_active();

void trackers_scanner_add_tracker_profile(tracker_profile_t** profiles,
                                          int* num_profiles,
                                          uint8_t mac_address[6],
                                          int rssi,
                                          char* name);

int trackers_scanner_find_profile_by_mac(tracker_profile_t* profiles,
                                         int num_profiles,
                                         uint8_t mac_address[6]);
#endif  // TRACKERS_SCANNER_H
