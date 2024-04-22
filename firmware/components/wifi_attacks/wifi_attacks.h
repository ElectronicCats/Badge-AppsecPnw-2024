#include "esp_wifi.h"
#ifndef WIFI_ATTACKS_MODULE_H
  #define WIFI_ATTACKS_MODULE_H
  #define TAG_WIFI_ATTACK_MODULE "module:wifi_attacks"
  #define DEFAULT_SCAN_LIST_SIZE CONFIG_SCAN_MAX_AP

/**
 * @brief Structure to store deauth frame data
 *
 */
typedef struct {
  const uint8_t* frame_buffer;
  int size;
  int task_id;
} deauth_frame_t;

/**
 * @brief Enum to define the types of attacks
 *
 */
typedef enum {
  WIFI_ATTACK_BROADCAST = 0,
  WIFI_ATTACK_ROGUE_AP,
  WIFI_ATTACK_COMBINE
} wifi_attacks_types_t;

/**
 * @brief Start the wifi attacks module
 *
 * @param attack_type The wifi attack type
 * @param ap_target The pointer  to the target AP to attack
 */
void wifi_attack_handle_attacks(wifi_attacks_types_t attack_type,
                                wifi_ap_record_t* ap_target);
/**
 * @brief Stop the wifi attacks module
 *
 */
void wifi_attacks_module_stop();

/**
 * @brief Start the broadcast attack
 *
 * @param ap_target target AP to attack
 */
void wifi_attack_brod_send_deauth_frame(wifi_ap_record_t* ap_target);
/**
 * @brief Start the Rogue AP  attack
 *
 * @note BSSID is MAC address of APs Wi-Fi interface
 *
 * @param ap_record target AP that will be cloned/duplicated
 */
void wifi_attack_rogueap(const wifi_ap_record_t* ap_record);
#endif  // WIFI_ATTACKS_MODULE_H
