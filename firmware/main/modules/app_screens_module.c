#include "drivers/oled_ssd1306_driver.h"
#include "esp_log.h"
#include "modules/app_screen_module.h"
#include "modules/ble/airtags_module.h"
#include "modules/menu_screens_modules.h"

uint8_t bluetooth_devices_count;
app_screen_state_information_t app_screen_state_information = {
    .in_app = false,
    .app_selected = 0,
};
int airtag_count = 0;
int device_selection = 0;
bool is_displaying = false;
bool is_modal_displaying = false;
tracker_profile_t* scanned_airtags = NULL;
TaskHandle_t screen_app_task_display_records = NULL;

void app_screen_state_machine_init(int app_selected) {
  ESP_LOGI(TAG_APP_SCREEN_MODULE, "Initializing screen state machine");
  app_screen_state_information.app_selected = app_selected;

  module_keyboard_update_state(true, app_screen_state_machine);
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  app_screen_select_app();
};

void app_screen_select_app() {
  switch (app_screen_state_information.app_selected) {
    case BLUETOOTH_MENU_AIRTAGS_SCAN:
      bluetooth_devices_count = 0;
      trackers_scanner_register_cb(screen_app_display_bluetooth_scanner);
      screen_app_task_start_display_devices();
      trackers_scanner_start();
      break;
    default:
      break;
  }
}

void app_screen_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;

  ESP_LOGI(TAG_APP_SCREEN_MODULE, "Game engine state machine from team: %d %d",
           button_name, button_event);
  switch (app_screen_state_information.app_selected) {
    case BLUETOOTH_MENU_AIRTAGS_SCAN:
      ESP_LOGI(TAG_APP_SCREEN_MODULE, "Bluetooth scanner entered");
      switch (button_name) {
        case BUTTON_LEFT:
          ESP_LOGI(TAG_APP_SCREEN_MODULE, "Button left pressed");
          if (is_modal_displaying) {
            is_modal_displaying = false;
            oled_driver_clear(OLED_DISPLAY_NORMAL);
            oled_driver_display_text(0, "Trackers Scanner", 17,
                                     OLED_DISPLAY_INVERTED);
            break;
          }

          screen_app_task_stop_display_devices();
          trackers_scanner_stop();
          module_keyboard_update_state(false, NULL);
          screen_module_exit_submenu();
          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_APP_SCREEN_MODULE,
                   "Button right pressed - Option selected: %d",
                   device_selection);
          if (is_modal_displaying) {
            break;
          }
          is_modal_displaying = true;
          screen_app_display_modal(scanned_airtags[device_selection]);
          break;
        case BUTTON_UP:
          ESP_LOGI(TAG_APP_SCREEN_MODULE, "Button up pressed");
          device_selection = (device_selection == 0) ? 0 : device_selection - 1;
          break;
        case BUTTON_DOWN:
          ESP_LOGI(TAG_APP_SCREEN_MODULE, "Button down pressed");
          device_selection = (device_selection == airtag_count)
                                 ? device_selection
                                 : device_selection + 1;
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void screen_app_display_bluetooth_scanner(tracker_profile_t record) {
  int has_device = trackers_scanner_find_profile_by_mac(
      scanned_airtags, airtag_count, record.mac_address);
  if (has_device == -1) {
    trackers_scanner_add_tracker_profile(&scanned_airtags, &airtag_count,
                                         record.mac_address, record.rssi,
                                         record.name);
  } else {
    scanned_airtags[has_device].rssi = record.rssi;
    if (is_modal_displaying) {
      screen_app_display_modal(scanned_airtags[device_selection]);
    }
  }
}

void screen_app_task_start_display_devices() {
  is_displaying = true;
  xTaskCreate(screen_app_task_display_devices, "display_records", 2048, NULL,
              10, &screen_app_task_display_records);
}

void screen_app_task_stop_display_devices() {
  is_displaying = false;
  vTaskSuspend(screen_app_task_display_records);
}

void screen_app_task_display_devices() {
  // #TODO: Implement the scrolling of the devices
  char* name_str = (char*) malloc(50);
  oled_driver_display_text(0, "Trackers Scanner", 17, OLED_DISPLAY_INVERTED);
  while (is_displaying) {
    if (!is_modal_displaying) {
      int started_page = 2;
      for (int i_device = 0; i_device < airtag_count; i_device++) {
        oled_driver_clear_line(started_page, OLED_DISPLAY_NORMAL);
        sprintf(name_str, "%s RSSI: %d dBM", scanned_airtags[i_device].name,
                scanned_airtags[i_device].rssi);
        oled_driver_display_text_splited(name_str, &started_page,
                                         (device_selection == i_device)
                                             ? OLED_DISPLAY_INVERTED
                                             : OLED_DISPLAY_NORMAL);
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  free(name_str);
}

void screen_app_display_modal(tracker_profile_t profile) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  int started_page = 1;
  char* name = (char*) malloc(MAX_LINE_CHAR);
  char* rssi = (char*) malloc(MAX_LINE_CHAR);
  char* mac_addrs = (char*) malloc(20);
  // char*    mac_addrs_s  = (char*) malloc(10);
  char* str_adv_data = (char*) malloc(64);

  memset(str_adv_data, 0, 64);
  sprintf(name, "%s", profile.name);
  sprintf(rssi, "RSSI: %d dBM", profile.rssi);
  sprintf(mac_addrs, "%02X:%02X:%02X:%02X:%02X%02X", profile.mac_address[0],
          profile.mac_address[1], profile.mac_address[2],
          profile.mac_address[3], profile.mac_address[4],
          profile.mac_address[5]);
  // sprintf(mac_addrs_s, "%02X:%02X:%02X ", profile.mac_address[3],
  // profile.mac_address[4],
  //         profile.mac_address[5]);

  ESP_LOGI(
      TAG_APP_SCREEN_MODULE,
      "Displaying modal with the following data %d:", profile.adv_data_length);
  sprintf(str_adv_data, "ADV: ");
  for (int i = 96; i < 112; i++) {
    sprintf(str_adv_data + strlen(str_adv_data), "%02X ", profile.adv_data[i]);
  }
  // esp_log_buffer_hex(TAG_APP_SCREEN_MODULE, adv_data, 62);
  // oled_driver_display_text_center(0, vendor, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, name, OLED_DISPLAY_NORMAL);
  // oled_driver_display_text_splited(name, &started_page, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_splited(rssi, &started_page, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(started_page, "MAC Address",
                                  OLED_DISPLAY_NORMAL);
  started_page++;
  oled_driver_display_text_center(started_page, mac_addrs, OLED_DISPLAY_NORMAL);
  started_page++;
  // oled_driver_display_text_center(started_page,mac_addrs_s,
  // OLED_DISPLAY_NORMAL); started_page++;
  oled_driver_display_text_splited(str_adv_data, &started_page,
                                   OLED_DISPLAY_NORMAL);
  free(name);
  free(rssi);
  free(mac_addrs);
  // free(mac_addrs_s);
  free(str_adv_data);
}
