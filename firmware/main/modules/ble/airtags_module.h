#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#ifndef AIRTAGS_MODULE_H
  #define AIRTAGS_MODULE_H
  #define TAG_BLE_CLIENT_MODULE          "AIRTAGS_MODULE"
  #define AIRTAG_REMOTE_SERVICE_UUID     0x00FF
  #define AIRTAG_REMOTE_NOTIFY_CHAR_UUID 0xFF01
  #define AIRTAG_PROFILE_NUM             1
  #define AIRTAG_PROFILE_A_APP_ID        0
  #define AIRTAG_INVALID_HANDLE          0
  #define AIRTAG_SCAN_DURATION           60

struct gattc_profile_inst {
  esp_gattc_cb_t gattc_cb;
  uint16_t gattc_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_start_handle;
  uint16_t service_end_handle;
  uint16_t char_handle;
  esp_bd_addr_t remote_bda;
};

/**
 * @brief Struct contain the GATT Client profile instance
 */
typedef struct {
  esp_gattc_cb_t gattc_cb;
  uint16_t gattc_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_start_handle;
  uint16_t service_end_handle;
  uint16_t char_handle;
  esp_bd_addr_t remote_bda;
} gattc_profile_inst;

typedef struct {
  uint8_t mac_address[6];
  int rssi;
  const char* name;
  bool is_airtag;
  uint16_t count;
  bool has_finished;
} bluetooth_scanner_record_t;

typedef struct {
  uint8_t mac_address[6];
  int rssi;
  char* name;
} airtag_profile_t;

typedef void (*bluetooth_scanner_cb_t)(bluetooth_scanner_record_t record);

void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
void esp_gattc_cb(esp_gattc_cb_event_t event,
                  esp_gatt_if_t gattc_if,
                  esp_ble_gattc_cb_param_t* param);
void gattc_profile_event_handler(esp_gattc_cb_event_t event,
                                 esp_gatt_if_t gattc_if,
                                 esp_ble_gattc_cb_param_t* param);

void bluetooth_scanner_init();
void bluetooth_scanner_register_cb(bluetooth_scanner_cb_t cb);
void bluetooth_scanner_start();
void bluetooth_scanner_stop();
void bluetooth_scanner_deinit();
bool bluetooth_scanner_is_active();

void add_profile(airtag_profile_t** profiles,
                 int* num_profiles,
                 uint8_t mac_address[6],
                 int rssi,
                 const char* name);
#endif  // AIRTAGS_MODULE_H
