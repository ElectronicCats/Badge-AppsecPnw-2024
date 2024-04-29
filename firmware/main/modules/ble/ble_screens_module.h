#include "trackers_scanner.h"
#ifndef BLE_SCREENS_MODULE_H
  #define BLE_SCREENS_MODULE_H

void ble_screens_display_trackers_profiles(tracker_profile_t* trackers_scanned,
                                           int trackers_count,
                                           int device_selection);
void ble_screens_display_modal_trackers_profile(tracker_profile_t profile);
#endif  // BLE_SCREENS_MODULE_H
