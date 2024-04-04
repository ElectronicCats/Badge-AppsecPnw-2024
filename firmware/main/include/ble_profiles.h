#ifndef BLE_PROFILES_H
#define BLE_PROFILES_H
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "engine.h"

#define TAG_BLE "EC_APPSECPWN_BLE"
#define BOARD_RED_DEVICE_NAME "EC_APPSECPWN_RED"
// Device Profiles
#define DEVICE_PROFILES 1
#define DEVICE_PROFILE 0

#define GATTS_SERVICE_UUID_USERNAME 0x00FF
#define GATTS_CHAR_UUID_USERNAME 0xFF01
#define GATTS_DESCR_UUID_USERNAME 0x3333
#define GATTS_NUM_HANDLE_USERNAME 4
#define BOARD_MANUFACTURER_DATA_LEN 17
#define GATTS_DEVICE_CHAR_VAL_LEN_MAX 0x80  // 128 bytes
#define PREPARE_BUF_MAX_SIZE 1024

#define adv_config_flag (1 << 0)
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
}gatts_profile_inst;

extern struct gatts_profile_inst gatts_profile_tab[DEVICE_PROFILES];
static prepare_type_env_t a_prepare_write_env;

extern PlayerInformation red_player_information;

void board_write_event(esp_gatt_if_t gatts_if, prepare_type_env_t* prepare_write_env, esp_ble_gatts_cb_param_t* param);
void board_exce_write_event(prepare_type_env_t* prepare_write_env, esp_ble_gatts_cb_param_t* param);
extern void gatts_profiles_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);

extern void get_player_array(esp_gatt_rsp_t* response_information, PlayerInformation player);
extern void get_player_array_uint8(uint8_t* response_information, PlayerInformation player);
extern void get_profile_owasp_array(esp_gatt_rsp_t* response_information, OWASPProfile *profile);
extern void get_profile_owasp_uint8(uint8_t* response_information, OWASPProfile *profile);

void init_ble_task(void);
void stop_ble_task(void);
#endif // BLE_PROFILES_H