#include <string.h>
#include "drivers/oled_ssd1306_driver.h"
#include "esp_wifi.h"
#include "modules/wifi/wifi_bitmaps.h"

int max_records_to_display = 7;

static const char* wifi_auth_modes[] = {"OPEN",
                                        "WEP",
                                        "WPA_PSK",
                                        "WPA2_PSK",
                                        "WPA_WPA2_PSK",
                                        "ENTERPRISE",
                                        "WPA3_PSK",
                                        "WPA2/3_PSK",
                                        "WAPI_PSK",
                                        "OWE",
                                        "WPA3_ENT_192",
                                        "WPA3_EXT_PSK",
                                        "WPA3EXTPSK_MIXED",
                                        "MAX"};

static const char* wifi_cipher_types[] = {
    "NONE",        "WEP40",       "WEP104", "TKIP", "CCMP",
    "TKIP_CCMP",   "AES_CMAC128", "SMS4",   "GCMP", "GCMP256",
    "AES_GMAC128", "AES_GMAC256", "UNKNOWN"};

void wifi_screens_module_scanning(void) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "SCANNING", OLED_DISPLAY_NORMAL);
  while (true) {
    for (int i = 0; i < wifi_bitmap_allArray_LEN; i++) {
      oled_driver_display_bitmap(48, 16, wifi_bitmap_allArray[i], 32, 32,
                                 OLED_DISPLAY_NORMAL);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void wifi_screens_module_animate_attacking(wifi_ap_record_t* ap_record) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  char* ssid = (char*) malloc(33);
  memset(ssid, 0, 33);
  sprintf(ssid, "%s", (char*) ap_record->ssid);

  oled_driver_display_text_center(0, "TARGETING", OLED_DISPLAY_INVERTED);
  oled_driver_display_text_center(1, ssid, OLED_DISPLAY_INVERTED);

  while (true) {
    for (int i = 0; i < wifi_bitmap_allArray_LEN; i++) {
      oled_driver_display_bitmap(48, 16, wifi_bitmap_allArray[i], 32, 32,
                                 OLED_DISPLAY_NORMAL);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  free(ssid);
}

void wifi_screens_module_display_scanned_networks(wifi_ap_record_t* ap_records,
                                                  int scanned_records,
                                                  int current_option) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "Select a network", OLED_DISPLAY_NORMAL);

  for (int i = current_option; i < (max_records_to_display + current_option);
       i++) {
    if (i >= scanned_records) {
      break;
    }
    if (i == current_option) {
      char* prefix = "> ";
      char item_text[strlen(prefix) + strlen((char*) ap_records[i].ssid) + 1];
      strcpy(item_text, prefix);
      strcat(item_text, (char*) ap_records[i].ssid);
      oled_driver_display_text((i + 1) - current_option, item_text,
                               OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_text((i + 1) - current_option,
                               (char*) ap_records[i].ssid, OLED_DISPLAY_NORMAL);
    }
  }
}

void wifi_screens_module_display_details_network(wifi_ap_record_t* ap_record,
                                                 int page) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  char* ssid = (char*) malloc(33);
  memset(ssid, 0, 33);
  sprintf(ssid, "%s", (char*) ap_record->ssid);
  oled_driver_display_text_center(0, ssid, OLED_DISPLAY_INVERTED);

  if (page == 0) {
    char* bssid = (char*) malloc(20);
    char* rssi_channel = (char*) malloc(MAX_LINE_CHAR);
    char* auth_mode = (char*) malloc(20);

    sprintf(auth_mode, "%s", wifi_auth_modes[ap_record->authmode]);
    sprintf(rssi_channel, "%d dBm   %d", ap_record->rssi, ap_record->primary);
    sprintf(bssid, "%02X:%02X:%02X:%02X:%02X%02X", ap_record->bssid[0],
            ap_record->bssid[1], ap_record->bssid[2], ap_record->bssid[3],
            ap_record->bssid[4], ap_record->bssid[5]);
    oled_driver_display_text_center(2, "RSSI   PRIM CH", OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(3, rssi_channel, OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(4, "BSSID", OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(5, bssid, OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(6, "AUTH MODE", OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(7, auth_mode, OLED_DISPLAY_NORMAL);
    free(bssid);
    free(rssi_channel);
    free(auth_mode);
  } else {
    char* pairwise_cipher = (char*) malloc(20);
    char* group_cipher = (char*) malloc(20);

    sprintf(pairwise_cipher, "%s",
            wifi_cipher_types[ap_record->pairwise_cipher]);
    sprintf(group_cipher, "%s", wifi_cipher_types[ap_record->group_cipher]);

    oled_driver_display_text_center(2, "PAIRWISE CIPHER", OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(3, pairwise_cipher, OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(5, "GROUP CIPHER", OLED_DISPLAY_NORMAL);
    oled_driver_display_text_center(6, group_cipher, OLED_DISPLAY_NORMAL);

    free(pairwise_cipher);
    free(group_cipher);
  }
  free(ssid);
}

void wifi_screens_module_display_attack_selector(char* attack_options[],
                                                 int list_count,
                                                 int current_option) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "Select Attack", OLED_DISPLAY_NORMAL);
  for (int i = 0; i < list_count; i++) {
    if (attack_options[i] == NULL) {
      break;
    }

    if (i == current_option) {
      char* prefix = "> ";
      char item_text[strlen(prefix) + strlen(attack_options[i]) + 1];
      strcpy(item_text, prefix);
      strcat(item_text, attack_options[i]);
      oled_driver_display_text(i + 1, item_text, OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_text(i + 1, attack_options[i], OLED_DISPLAY_NORMAL);
    }
  }
}

void wifi_screens_module_display_captive_pass(char* ssid,
                                              char* user,
                                              char* pass) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "Captive Portal", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(1, "SSID", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(2, ssid, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(3, "PASS", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(4, pass, OLED_DISPLAY_INVERTED);
}

void wifi_screens_module_display_captive_user_pass(char* ssid,
                                                   char* user,
                                                   char* pass) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "Captive Portal", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(1, "SSID", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(2, ssid, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(3, "USER", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(4, user, OLED_DISPLAY_INVERTED);
  oled_driver_display_text_center(5, "PASS", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(6, pass, OLED_DISPLAY_INVERTED);
}

void wifi_screens_module_display_captive_selector(char* attack_options[],
                                                  int list_count,
                                                  int current_option) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "Select Portal", OLED_DISPLAY_NORMAL);
  for (int i = 0; i < list_count; i++) {
    if (attack_options[i] == NULL) {
      break;
    }

    if (i == current_option) {
      char* prefix = "> ";
      char item_text[strlen(prefix) + strlen(attack_options[i]) + 1];
      strcpy(item_text, prefix);
      strcat(item_text, attack_options[i]);
      oled_driver_display_text(i + 1, item_text, OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_text(i + 1, attack_options[i], OLED_DISPLAY_NORMAL);
    }
  }
}
