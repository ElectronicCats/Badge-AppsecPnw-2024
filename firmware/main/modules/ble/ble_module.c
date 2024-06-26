#include "ble_module.h"
#include "bt_spam.h"
#include "drivers/oled_ssd1306_driver.h"
#include "esp_log.h"
#include "modules/ble/ble_screens_module.h"
#include "modules/led_events.h"
#include "modules/menu_screens_modules.h"
#include "trackers_scanner.h"

static app_screen_state_information_t app_screen_state_information = {
    .in_app = false,
    .app_selected = 0,
};
static int trackers_count = 0;
static int device_selection = 0;
static bool is_displaying = false;
static bool is_modal_displaying = false;
static tracker_profile_t* scanned_airtags = NULL;
static TaskHandle_t ble_task_display_records = NULL;
static TaskHandle_t ble_task_display_animation = NULL;

static void ble_module_app_selector();
static void ble_module_state_machine(button_event_t button_pressed);
static void ble_module_display_trackers_cb(tracker_profile_t record);
static void ble_module_task_start_trackers_display_devices();
static void ble_module_task_stop_trackers_display_devices();

void ble_module_begin(int app_selected) {
  ESP_LOGI(TAG_BLE_MODULE, "Initializing ble module screen state machine");
  app_screen_state_information.app_selected = app_selected;

  module_keyboard_update_state(true, ble_module_state_machine);
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  ble_module_app_selector();
};

static void ble_module_app_selector() {
  led_control_run_effect(led_control_ble_tracking);
  switch (app_screen_state_information.app_selected) {
    case BLUETOOTH_MENU_AIRTAGS_SCAN:

      trackers_scanner_register_cb(ble_module_display_trackers_cb);
      ble_module_task_start_trackers_display_devices();
      trackers_scanner_start();
      break;
    case BLUETOOTH_MENU_SPAM:
      xTaskCreate(ble_screens_display_scanning_animation, "ble_module_scanning",
                  4096, NULL, 5, &ble_task_display_animation);
      bt_spam_register_cb(ble_screens_display_scanning_text);
      bt_spam_app_main();
      break;
    default:
      break;
  }
}

static void ble_module_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;

  ESP_LOGI(TAG_BLE_MODULE, "BLE engine state machine from team: %d %d",
           button_name, button_event);
  switch (app_screen_state_information.app_selected) {
    case BLUETOOTH_MENU_AIRTAGS_SCAN:
      ESP_LOGI(TAG_BLE_MODULE, "Bluetooth scanner entered");
      switch (button_name) {
        case BUTTON_LEFT:

          if (button_event == BUTTON_LONG_PRESS_UP) {
            // ble_module_task_stop_trackers_display_devices();
            // trackers_scanner_stop();
            // module_keyboard_update_state(false, NULL);
            // screen_module_exit_submenu();
            // led_control_stop();
            led_control_stop();
            screen_module_set_screen(LAYER_BLUETOOTH_APPS);
            esp_restart();
          }
          ESP_LOGI(TAG_BLE_MODULE, "Button left pressed");
          if (is_modal_displaying) {
            is_modal_displaying = false;
            oled_driver_clear(OLED_DISPLAY_NORMAL);
            oled_driver_display_text_center(0, "Trackers Scanner",
                                            OLED_DISPLAY_INVERTED);
            break;
          }

          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_BLE_MODULE, "Button right pressed - Option selected: %d",
                   device_selection);
          if (is_modal_displaying) {
            break;
          }
          is_modal_displaying = true;
          ble_screens_display_modal_trackers_profile(
              scanned_airtags[device_selection]);
          break;
        case BUTTON_UP:
          ESP_LOGI(TAG_BLE_MODULE, "Button up pressed");
          device_selection = (device_selection == 0) ? 0 : device_selection - 1;
          break;
        case BUTTON_DOWN:
          ESP_LOGI(TAG_BLE_MODULE, "Button down pressed");
          device_selection = (device_selection == (trackers_count - 1))
                                 ? device_selection
                                 : device_selection + 1;
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    case BLUETOOTH_MENU_SPAM:
      switch (button_name) {
        case BUTTON_LEFT:
          // TODO: Fix this xD
          if (button_event == BUTTON_LONG_PRESS_UP) {
            // ble_module_task_stop_trackers_display_devices();
            // trackers_scanner_stop();
            // module_keyboard_update_state(false, NULL);
            // screen_module_exit_submenu();
            led_control_stop();
            screen_module_set_screen(LAYER_BLUETOOTH_APPS);
            esp_restart();
          }
          // ESP_LOGI(TAG_BLE_MODULE, "Button left pressed");
          // vTaskSuspend(ble_task_display_animation);
          // module_keyboard_update_state(false, NULL);
          // ESP_LOGI(TAG_BLE_MODULE, "Exiting bluetooth scanner 2");
          // screen_module_exit_submenu();
          break;
        case BUTTON_RIGHT:
        case BUTTON_UP:
        case BUTTON_DOWN:
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    default:
      break;
  }
}

static void ble_module_display_trackers_cb(tracker_profile_t record) {
  int has_device = trackers_scanner_find_profile_by_mac(
      scanned_airtags, trackers_count, record.mac_address);
  if (has_device == -1) {
    trackers_scanner_add_tracker_profile(&scanned_airtags, &trackers_count,
                                         record.mac_address, record.rssi,
                                         record.name);
  } else {
    scanned_airtags[has_device].rssi = record.rssi;
    if (is_modal_displaying) {
      ble_screens_display_modal_trackers_profile(
          scanned_airtags[device_selection]);
    }
  }
}

static void ble_module_create_task_trackers_display_devices() {
  while (is_displaying) {
    if (!is_modal_displaying) {
      ble_screens_display_trackers_profiles(scanned_airtags, trackers_count,
                                            device_selection);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static void ble_module_task_start_trackers_display_devices() {
  is_displaying = true;
  xTaskCreate(ble_module_create_task_trackers_display_devices,
              "display_records", 2048, NULL, 10, &ble_task_display_records);
}

static void ble_module_task_stop_trackers_display_devices() {
  is_displaying = false;
  vTaskSuspend(ble_task_display_records);
}
