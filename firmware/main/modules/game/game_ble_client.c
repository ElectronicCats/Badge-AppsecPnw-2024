
#include "modules/game/game_ble_client.h"
#include "bt_gattc.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "inttypes.h"
#include "modules/game_engine_module.h"

static void handle_bt_gattc_events(esp_gattc_cb_event_t event_type,
                                   esp_ble_gattc_cb_param_t* param);

void ble_client_task_begin() {
  gattc_scan_params_t scan_params = {
      .remote_filter_service_uuid =
          bt_gattc_set_default_ble_filter_service_uuid(),
      .remote_filter_char_uuid = bt_gattc_set_default_ble_filter_char_uuid(),
      .notify_descr_uuid = bt_gattc_set_default_ble_notify_descr_uuid(),
      .ble_scan_params = bt_gattc_set_default_ble_scan_params()};
  bt_gattc_set_ble_scan_params(&scan_params);
  bt_client_event_cb_t event_cb = {.handler_gattc_cb = handle_bt_gattc_events,
                                   .handler_gapc_cb = NULL};
  // bt_gattc_set_remote_device_name("EC_APPSECPWN_RED");
  bt_gattc_set_cb(event_cb);
  bt_gattc_task_begin();
}

void ble_client_task_stop() {
  bt_gattc_task_stop();
}

static void handle_bt_gattc_events(esp_gattc_cb_event_t event_type,
                                   esp_ble_gattc_cb_param_t* param) {
  ESP_LOGI(TAG_BLE_CLIENT_MODULE, "Event: %d", event_type);
  switch (event_type) {
    case ESP_GATTC_CONNECT_EVT:
      game_engine_cb_paired_devices(GAME_TEAM_BLUE);
      break;
    case ESP_GATTC_NOTIFY_EVT:
      char* ble_recived = (char*) param->notify.value;
      game_engine_handle_server_data(ble_recived);
      break;
    default:
      break;
  }
}
