#include <string.h>
#include "drivers/oled_ssd1306_driver.h"
#include "trackers_scanner.h"

void ble_screens_display_trackers_profiles(tracker_profile_t* trackers_scanned,
                                           int trackers_count,
                                           int device_selection) {
  char* name_str = (char*) malloc(50);
  oled_driver_display_text_center(0, "Trackers Scanner", OLED_DISPLAY_INVERTED);
  int started_page = 2;
  for (int i_device = 0; i_device < trackers_count; i_device++) {
    oled_driver_clear_line(started_page, OLED_DISPLAY_NORMAL);
    sprintf(name_str, "%s RSSI: %d dBM", trackers_scanned[i_device].name,
            trackers_scanned[i_device].rssi);
    oled_driver_display_text_splited(name_str, &started_page,
                                     (device_selection == i_device)
                                         ? OLED_DISPLAY_INVERTED
                                         : OLED_DISPLAY_NORMAL);
  }
  free(name_str);
}

void ble_screens_display_modal_trackers_profile(tracker_profile_t profile) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  int started_page = 1;
  char* name = (char*) malloc(MAX_LINE_CHAR);
  char* rssi = (char*) malloc(MAX_LINE_CHAR);
  char* mac_addrs = (char*) malloc(20);
  char* str_adv_data = (char*) malloc(64);

  memset(str_adv_data, 0, 64);
  sprintf(name, "%s", profile.name);
  sprintf(rssi, "RSSI: %d dBm", profile.rssi);
  sprintf(mac_addrs, "%02X:%02X:%02X:%02X:%02X%02X", profile.mac_address[0],
          profile.mac_address[1], profile.mac_address[2],
          profile.mac_address[3], profile.mac_address[4],
          profile.mac_address[5]);

  sprintf(str_adv_data, "ADV: ");
  for (int i = 96; i < 112; i++) {
    sprintf(str_adv_data + strlen(str_adv_data), "%02X ", profile.adv_data[i]);
  }
  oled_driver_display_text_center(0, name, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_splited(rssi, &started_page, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(started_page, "MAC Address",
                                  OLED_DISPLAY_NORMAL);
  started_page++;
  oled_driver_display_text_center(started_page, mac_addrs, OLED_DISPLAY_NORMAL);
  started_page++;
  oled_driver_display_text_splited(str_adv_data, &started_page,
                                   OLED_DISPLAY_NORMAL);
  free(name);
  free(rssi);
  free(mac_addrs);
  free(str_adv_data);
}
