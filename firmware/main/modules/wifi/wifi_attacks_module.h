#include "esp_wifi.h"
#ifndef WIFI_ATTACKS_MODULE_H
  #define WIFI_ATTACKS_MODULE_H
  #define TAG_WIFI_ATTACK_MODULE "module:wifi_attacks"
  #define DEFAULT_SCAN_LIST_SIZE CONFIG_SCAN_MAX_AP

typedef struct {
  const uint8_t* frame_buffer;
  int size;
  int task_id;
} deauth_frame_t;

typedef enum {
  WIFI_ATTACK_BROADCAST = 0,
  WIFI_ATTACK_ROGUE_AP,
  WIFI_ATTACK_COMBINE
} wifi_attacks_types_t;

void wifi_attack_handle_attacks(int attack_type, wifi_ap_record_t* ap_target);
void wifi_attacks_module_stop();
void wifi_attack_brod_send_deauth_frame(wifi_ap_record_t* ap_target);
/**
 * @note BSSID is MAC address of APs Wi-Fi interface
 *
 * @param ap_record target AP that will be cloned/duplicated
 */
void wifi_attack_rogueap(const wifi_ap_record_t* ap_record);
#endif  // WIFI_ATTACKS_MODULE_H
