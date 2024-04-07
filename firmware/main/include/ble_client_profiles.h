#ifndef BLE_CLIENT_PROFILES_H
#define BLE_CLIENT_PROFILES_H
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "engine.h"

#define TAG_BLE "EC_APPSECPWN_BLE_CLIENT"

#define REMOTE_BOARD_ID "EC_APPSECPWN_RED"
//GATT Client
#define REMOTE_SERVICE_USERNAME_UUID 0x00FF
#define REMOTE_NOTIFY_CHAR_UUID    0xFF01
#define DEVICE_PROFILE 0
#define INVALID_HANDLE   0
#define DEVICE_PROFILES 1

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

typedef struct {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
}gattc_profile_inst;

extern struct gattc_profile_inst gattc_profile_tab[DEVICE_PROFILES];

void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
extern void gattc_profiles_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);


void init_ble_client_task(void);
#endif // BLE_CLIENT_PROFILES_H