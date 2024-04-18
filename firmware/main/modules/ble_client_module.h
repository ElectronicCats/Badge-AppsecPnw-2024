#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#ifndef BLE_CLIENT_MODULE_H
  #define BLE_CLIENT_MODULE_H
  #define TAG_BLE_CLIENT_MODULE "BLE_CLIENT_MODULE"
  #define REMOTE_BOARD_ID       "EC_APPSECPWN_RED"
  // GATT Client
  #define REMOTE_SERVICE_USERNAME_UUID 0x00FF
  #define REMOTE_NOTIFY_CHAR_UUID      0xFF01
  #define DEVICE_PROFILE               0
  #define INVALID_HANDLE               0
  #define DEVICE_PROFILES              1

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

extern struct gattc_profile_inst gattc_profile_tab[DEVICE_PROFILES];

/**
 * @brief GATT Client event handler
 *
 * @param event The GATT Client event
 * @param gattc_if The GATT interface
 *
 * @return void
 */
void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
/**
 * @brief GATT Client event handler
 *
 * @param event The GATT Client event
 * @param gattc_if The GATT interface
 * @param param The GATT Client parameters
 *
 * @return void
 */
void esp_gattc_cb(esp_gattc_cb_event_t event,
                  esp_gatt_if_t gattc_if,
                  esp_ble_gattc_cb_param_t* param);
/**
 * @brief Send data to the remote device
 *
 * @param data The data to send
 * @param length The length of the data
 *
 * @return void
 */
void ble_client_send_data(uint8_t* data, int length);
/**
 * @brief GATT Client event handler
 *
 * @param event The GATT Client event
 * @param gattc_if The GATT interface
 * @param param The GATT Client parameters
 *
 * @return void
 */
void ble_client_gatt_profiles_event_handler(esp_gattc_cb_event_t event,
                                            esp_gatt_if_t gattc_if,
                                            esp_ble_gattc_cb_param_t* param);

/**
 * @brief Initialize the GATT Client profile
 *
 * @return void
 */
void ble_client_task_begin(void);
/**
 * @brief Stop the GATT Client profile
 *
 * @return void
 */
void ble_client_task_stop(void);
#endif  // BLE_CLIENT_MODULE_H
