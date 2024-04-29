
#include "modules/game/game_ble_server.h"
#include "bt_gatts.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "inttypes.h"
#include "modules/game_engine_module.h"

static void hadle_bt_gatts_events(esp_gatts_cb_event_t event_type,
                                  esp_ble_gatts_cb_param_t* param);

void ble_server_task_begin() {
  bt_server_props_t props = {.device_name = "EC_APPSECPWN_RED",
                             .manufacturer_data = (uint8_t*) "Electronic Cats"};
  gatts_adv_params_t adv_params = {
      .adv_data = bt_gatts_set_default_ble_adv_data(),
      .scan_rsp_data = bt_gatts_set_default_ble_scan_rsp(),
      .adv_params = bt_gatts_set_default_ble_adv_params(),
      .char_val = bt_gatts_set_default_char_val(),
      .bt_props = props};
  bt_gatts_set_ble_adv_data_params(&adv_params);
  bt_server_event_cb_t event_cb = {.handler_gatt_cb = hadle_bt_gatts_events,
                                   .handler_gap_cb = NULL};
  bt_gatts_set_cb(event_cb);
  bt_gatts_task_begin();
}

void ble_server_task_stop() {
  bt_gatts_task_stop();
}

static void hadle_bt_gatts_events(esp_gatts_cb_event_t event_type,
                                  esp_ble_gatts_cb_param_t* param) {
  ESP_LOGI("BLE_SERVER_MODULE", "Event: %d", event_type);
  switch (event_type) {
    case ESP_GATTS_CONNECT_EVT:
      game_engine_cb_paired_devices(GAME_TEAM_RED);
      break;
    case ESP_GATTS_WRITE_EVT:
      char* ble_recived = (char*) param->write.value;
      game_engine_handle_server_data(ble_recived);
      break;
    default:
      break;
  }
}
