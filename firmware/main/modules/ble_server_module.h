#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#ifndef BLE_SERVER_MODULE_H
  #define BLE_SERVER_MODULE_H
  #define TAG_BLE_SERVER_MODULE "BLE_SERVER_MODULE"
  #define BOARD_RED_DEVICE_NAME "EC_APPSECPWN_RED"
  #define DEVICE_PROFILES       1
  #define DEVICE_PROFILE        0

  #define GATTS_SERVICE_UUID_USERNAME   0x00FF
  #define GATTS_CHAR_UUID_USERNAME      0xFF01
  #define GATTS_DESCR_UUID_USERNAME     0x3333
  #define GATTS_NUM_HANDLE_USERNAME     4
  #define BOARD_MANUFACTURER_DATA_LEN   17
  #define GATTS_DEVICE_CHAR_VAL_LEN_MAX 0x80  // 128 bytes
  #define PREPARE_BUF_MAX_SIZE          1024

  #define adv_config_flag      (1 << 0)
  #define scan_rsp_config_flag (1 << 1)

typedef struct {
  uint8_t* prepare_buf;
  int prepare_len;
} prepare_type_env_t;

struct gatts_profile_inst {
  esp_gatts_cb_t gatts_cb;
  uint16_t gatts_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_handle;
  esp_gatt_srvc_id_t service_id;
  uint16_t char_handle;
  esp_bt_uuid_t char_uuid;
  esp_gatt_perm_t perm;
  esp_gatt_char_prop_t property;
  uint16_t descr_handle;
  esp_bt_uuid_t descr_uuid;
};

/**
 * @brief Struct contain the GATT Server profile instance
 */
typedef struct {
  esp_gatts_cb_t gatts_cb;
  uint16_t gatts_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_handle;
  esp_gatt_srvc_id_t service_id;
  uint16_t char_handle;
  esp_bt_uuid_t char_uuid;
  esp_gatt_perm_t perm;
  esp_gatt_char_prop_t property;
  uint16_t descr_handle;
  esp_bt_uuid_t descr_uuid;
} gatts_profile_inst;

extern struct gatts_profile_inst gatts_profile_tab[DEVICE_PROFILES];
static prepare_type_env_t a_prepare_write_env;

/**
 * @brief GATT Server write event handler
 *
 * @param gatts_if The GATT interface
 * @param prepare_write_env The prepare write environment
 * @param param The GATT Server parameters
 *
 * @return void
 */
void ble_server_write_event(esp_gatt_if_t gatts_if,
                            prepare_type_env_t* prepare_write_env,
                            esp_ble_gatts_cb_param_t* param);

/**
 * @brief GATT Server write event handler
 *
 * @param prepare_write_env The prepare write environment
 * @param param The GATT Server parameters
 *
 * @return void
 */
void ble_server_exce_write_event(prepare_type_env_t* prepare_write_env,
                                 esp_ble_gatts_cb_param_t* param);

/**
 * @brief GATT Server event handler
 *
 * @param event The GATT Server event
 * @param gatts_if The GATT interface
 * @param param The GATT Server parameters
 *
 * @return void
 */
void ble_server_gatt_profiles_event_handler(esp_gatts_cb_event_t event,
                                            esp_gatt_if_t gatts_if,
                                            esp_ble_gatts_cb_param_t* param);
/**
 * @brief Send data to the remote device
 *
 * @param data The data to send
 * @param length The length of the data
 *
 * @return void
 */
void ble_server_send_data(uint8_t* data, int length);
/**
 * @brief Initialize the GATT Server profile
 *
 * @return void
 */
void ble_server_task_begin(void);
/**
 * @brief Stop the GATT Server profile
 *
 * @return void
 */
void ble_server_task_stop(void);
#endif  // BLE_SERVER_MODULE_H
