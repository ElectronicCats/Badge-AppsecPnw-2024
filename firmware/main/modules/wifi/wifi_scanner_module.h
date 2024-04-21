#include "esp_wifi.h"
#ifndef WIFI_SCANNER_MODULE_H
  #define WIFI_SCANNER_MODULE_H
  #define TAG_WIFI_SCANNER_MODULE "module:wifi_scanner"
  #define DEFAULT_SCAN_LIST_SIZE  CONFIG_SCAN_MAX_AP

/**
 * @brief Linked list of wifi_ap_record_t records.
 *
 */
typedef struct {
  uint16_t count;
  wifi_ap_record_t records[CONFIG_SCAN_MAX_AP];
} wifi_scanner_ap_records_t;

/**
 * @brief Switches ESP into scanning mode and stores result.
 *
 */
void wifi_scanner_scan_nearby_aps();

/**
 * @brief Returns current list of scanned APs.
 *
 * @return const wifi_scanner_ap_records_t*
 */
const wifi_scanner_ap_records_t* wifi_scanner_get_ap_records();

/**
 * @brief Returns AP record on given index
 *
 * @param index
 * @return const wifi_ap_record_t*
 */
const wifi_ap_record_t* wifi_scanner_get_ap_record(unsigned index);

void wifi_scanner_module_scan();
#endif  // WIFI_SCANNER_MODULE_H
