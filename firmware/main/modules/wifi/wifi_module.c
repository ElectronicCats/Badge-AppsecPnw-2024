
#include "modules/wifi/wifi_module.h"
#include "drivers/wifi_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "modules/cross_keyboard_module.h"
#include "modules/menu_screens_modules.h"
#include "modules/wifi/wifi_attacks_module.h"
#include "modules/wifi/wifi_scanner_module.h"
#include "modules/wifi/wifi_screens_module.h"
#include "string.h"

static wifi_module_t current_wifi_state;
static int current_option = 0;
static bool show_details = false;
bool valid_records = false;

static TaskHandle_t task_display_scanning = NULL;
static TaskHandle_t task_display_attacking = NULL;
wifi_scanner_ap_records_t* ap_records;
wifi_ap_record_t* ap_record_targeted;
static int index_targeted = 0;

static void scanning_task(void* pvParameters) {
  while (!valid_records) {
    wifi_scanner_module_scan();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  vTaskSuspend(task_display_scanning);
  wifi_screens_module_display_scanned_networks(
      ap_records->records, ap_records->count, current_option);
  vTaskDelete(NULL);
}

void wifi_module_begin(void) {
  ESP_LOGI(TAG_WIFI_MODULE, "Initializing WiFi module");
  module_keyboard_update_state(true, wifi_module_state_machine);

  current_wifi_state.state = WIFI_STATE_SCANNING;
  memset(&current_wifi_state.wifi_config, 0, sizeof(wifi_config_t));
  current_wifi_state.wifi_config = wifi_driver_access_point_begin();

  ESP_LOGI(TAG_WIFI_MODULE, "Initializing Task module");

  xTaskCreate(wifi_screens_module_scanning, "wifi_module_scanning", 4096, NULL,
              5, &task_display_scanning);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  xTaskCreate(scanning_task, "scan", 4096, NULL, 5, NULL);
  ap_records = wifi_scanner_get_ap_records();

  while (!valid_records) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if (ap_records->count > 10) {
      valid_records = true;
    }
    ap_records = wifi_scanner_get_ap_records();
  }

  current_wifi_state.state = WIFI_STATE_SCANNED;

  ESP_LOGI(TAG_WIFI_MODULE, "Displaying Records");
}

void wifi_module_handle_records() {}

void wifi_module_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;
  ESP_LOGI(TAG_WIFI_MODULE, "WiFi engine state machine from team: %d %d",
           button_name, button_event);

  switch (current_wifi_state.state) {
    case WIFI_STATE_SCANNING: {
      switch (button_name) {
        case BUTTON_LEFT: {
          if (button_event == BUTTON_DOUBLE_CLICK) {
            ESP_LOGI(TAG_WIFI_MODULE, "Button left pressed");
            module_keyboard_update_state(SCREEN_IN_NAVIGATION, NULL);
            screen_module_exit_submenu();
            wifi_driver_ap_stop();
            break;
          }
          break;
        }
        case BUTTON_RIGHT:
        case BUTTON_UP:
        case BUTTON_DOWN:
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    }
    case WIFI_STATE_SCANNED: {
      switch (button_name) {
        case BUTTON_LEFT: {
          if (button_event == BUTTON_DOUBLE_CLICK) {
            ESP_LOGI(TAG_WIFI_MODULE, "Button left pressed");
            module_keyboard_update_state(SCREEN_IN_NAVIGATION, NULL);
            screen_module_exit_submenu();
            wifi_driver_ap_stop();
            free(ap_record_targeted);
            break;
          }
          break;
        }
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_WIFI_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          show_details = true;
          current_wifi_state.state = WIFI_STATE_DETAILS;
          ap_record_targeted =
              (wifi_ap_record_t*) malloc(sizeof(wifi_ap_record_t));
          if (ap_record_targeted == NULL) {
            ESP_LOGE(TAG_WIFI_MODULE,
                     "Failed to allocate memory for ap_record_targeted");
            break;
          }
          memccpy(ap_record_targeted, &ap_records->records[current_option], 1,
                  sizeof(wifi_ap_record_t));
          index_targeted = current_option;

          current_option = 0;
          wifi_screens_module_display_details_network(
              &ap_records->records[index_targeted], current_option);
          break;
        case BUTTON_UP: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          ESP_LOGI(TAG_WIFI_MODULE, "Scanned records: %d", ap_records->count);
          wifi_screens_module_display_scanned_networks(
              &ap_records->records, ap_records->count, current_option);
          break;
        }
        case BUTTON_DOWN: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button down pressed");
          current_option = (current_option == (ap_records->count - 1))
                               ? current_option
                               : current_option + 1;
          ESP_LOGI(TAG_WIFI_MODULE, "Scanned records: %d", ap_records->count);
          wifi_screens_module_display_scanned_networks(
              &ap_records->records, ap_records->count, current_option);
          break;
        }
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    }
    case WIFI_STATE_DETAILS: {
      switch (button_name) {
        case BUTTON_LEFT: {
          if (button_event == BUTTON_DOUBLE_CLICK) {
            ESP_LOGI(TAG_WIFI_MODULE, "Button left pressed");
            module_keyboard_update_state(SCREEN_IN_NAVIGATION, NULL);
            screen_module_exit_submenu();
            wifi_driver_ap_stop();
            break;
          }
          show_details = false;
          current_wifi_state.state = WIFI_STATE_SCANNED;
          wifi_screens_module_display_scanned_networks(
              &ap_records->records, ap_records->count, current_option);
          break;
        }
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_WIFI_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          current_wifi_state.state = WIFI_STATE_ATTACK_SELECTOR;
          wifi_screens_module_display_attack_selector(wifi_attacks,
                                                      current_option);
          break;
        case BUTTON_UP: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button up pressed");
          current_option = 0;
          wifi_screens_module_display_details_network(
              &ap_records->records[index_targeted], current_option);
          break;
        }
        case BUTTON_DOWN: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button down pressed");
          current_option = 1;
          wifi_screens_module_display_details_network(
              &ap_records->records[index_targeted], current_option);
          break;
        }
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    }
    case WIFI_STATE_ATTACK_SELECTOR: {
      switch (button_name) {
        case BUTTON_LEFT: {
          if (button_event == BUTTON_DOUBLE_CLICK) {
            ESP_LOGI(TAG_WIFI_MODULE, "Button left pressed");
            module_keyboard_update_state(SCREEN_IN_NAVIGATION, NULL);
            screen_module_exit_submenu();
            wifi_driver_ap_stop();
            break;
          }
          show_details = false;
          current_wifi_state.state = WIFI_STATE_SCANNED;
          wifi_screens_module_display_scanned_networks(
              &ap_records->records, ap_records->count, current_option);
          break;
        }
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_WIFI_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          current_wifi_state.state = WIFI_STATE_ATTACK;
          wifi_attack_handle_attacks(current_option,
                                     &ap_records->records[index_targeted]);
          xTaskCreate(wifi_screens_module_attacking, "wifi_module_scanning",
                      4096, NULL, 5, &task_display_attacking);
          break;
        case BUTTON_UP: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          wifi_screens_module_display_attack_selector(wifi_attacks,
                                                      current_option);
          break;
        }
        case BUTTON_DOWN: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button down pressed");
          current_option =
              (current_option == (3 - 1)) ? current_option : current_option + 1;
          wifi_screens_module_display_attack_selector(wifi_attacks,
                                                      current_option);
          break;
        }
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    }
    case WIFI_STATE_ATTACK: {
      switch (button_name) {
        case BUTTON_LEFT: {
          if (button_event == BUTTON_DOUBLE_CLICK) {
            ESP_LOGI(TAG_WIFI_MODULE, "Button left pressed");
            module_keyboard_update_state(SCREEN_IN_NAVIGATION, NULL);
            screen_module_exit_submenu();
            wifi_attacks_module_stop();
            wifi_driver_ap_stop();
            vTaskSuspend(task_display_attacking);
            break;
          }
          wifi_attacks_module_stop();
          vTaskSuspend(task_display_attacking);

          show_details = false;
          current_wifi_state.state = WIFI_STATE_SCANNED;
          wifi_screens_module_display_scanned_networks(
              &ap_records->records, ap_records->count, current_option);
          break;
        }
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_WIFI_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          break;
        case BUTTON_UP: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          break;
        }
        case BUTTON_DOWN: {
          ESP_LOGI(TAG_WIFI_MODULE, "Button down pressed");
          current_option =
              (current_option == (3 - 1)) ? current_option : current_option + 1;
          break;
        }
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    }
    default:
      break;
  }
}
