#include "modules/menu_screens_modules.h"
#include <stdlib.h>
#include <string.h>
#include "drivers/oled_ssd1306_driver.h"
#include "esp_log.h"
#include "modules/game_engine_module.h"

screen_module_layer_t previous_layer;
screen_module_layer_t current_layer;
uint8_t selected_item;
int submenu_items = 0;

static app_state_t app_state = {
    .in_app = false,
    .app_handler = NULL,
};

void screen_module_begin() {
  oled_driver_init();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
}

char** screen_module_add_empty_strings(char** array, int length) {
  char** newArray = malloc((length + 2) * sizeof(char*));

  // Add the empty string at the beginning
  newArray[0] = strdup("");

  // Copy the original array
  for (int i = 0; i < length; i++) {
    newArray[i + 1] = strdup(array[i]);
  }

  // Add the empty string at the end
  newArray[length + 1] = strdup("");

  submenu_items = length + 2;

  return newArray;
}

char** screen_module_get_menu_items() {
  submenu_items = 0;
  char** submenu = menu_items[current_layer];
  if (submenu != NULL) {
    while (submenu[submenu_items] != NULL) {
      submenu_items++;
    }
  }

  if (submenu_items == 0) {
    return NULL;
  }

  return screen_module_add_empty_strings(menu_items[current_layer],
                                         submenu_items);
}

void screen_module_display_menu_items(char** items) {
  // Show only 3 options at a time in the following order:
  // Page 1: Option 1
  // Page 3: Option 2 -> selected option
  // Page 5: Option 3
  int page = 1;
  char* terminal_user = "appsec@pwn";
  oled_driver_display_text(page, terminal_user, strlen(terminal_user),
                           OLED_DISPLAY_NORMAL);
  page++;
  int items_list = submenu_items - 2;
  // if (items[i + 1] == NULL) {
  //     break;

  for (int i = 0; i < items_list; i++) {
    char* name = (char*) malloc(20);
    ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Item: %s", items[i]);

    strcpy(name, items[i + 1]);
    if (selected_item == i) {
      char* prefix = "> ";
      char item_text[strlen(prefix) + strlen(name) + 1];
      strcpy(item_text, prefix);
      strcat(item_text, name);
      oled_driver_display_text(page, item_text, strlen(item_text),
                               OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_text(page, name, strlen(name), OLED_DISPLAY_NORMAL);
    }
    page += 2;
  }
}

void screen_module_display_menu() {
  char** items = screen_module_get_menu_items();
  if (items == NULL) {
    ESP_LOGW(TAG_MENU_SCREEN_MODULE, "Options is NULL");
    return;
  }
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  screen_module_display_menu_items(items);
}

void screen_module_set_main_menu() {
  previous_layer = LAYER_MAIN_MENU;
  current_layer = LAYER_MAIN_MENU;
  submenu_items = 0;
}

void screen_module_display_in_progress() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "In Progress", 13, OLED_DISPLAY_NORMAL);
}

void screen_module_increment_index_item() {
  if (submenu_items == 0) {
    return;
  }

  if (selected_item < MAX_MENU_ITEMS_PER_SCREEN - 1) {
    selected_item++;
  } else {
    selected_item = 0;
  }
}
void screen_module_decrement_index_item() {
  if (submenu_items == 0) {
    return;
  }

  if (selected_item > 0) {
    selected_item--;
  } else {
    selected_item = MAX_MENU_ITEMS_PER_SCREEN - 1;
  }
}

void screen_module_update_previous_layer() {
  switch (current_layer) {
    case LAYER_MAIN_MENU:
    case LAYER_APPLICATIONS:
    case LAYER_SETTINGS:
    case LAYER_ABOUT:
      previous_layer = LAYER_MAIN_MENU;
      break;
    case LAYER_WIFI_APPS:
    case LAYER_BLUETOOTH_APPS:
    case LAYER_ZIGBEE_APPS:
    case LAYER_THREAD_APPS:
    case LAYER_MATTER_APPS:
    case LAYER_GPS:
      previous_layer = LAYER_APPLICATIONS;
      break;
    case LAYER_ABOUT_VERSION:
    case LAYER_ABOUT_LICENSE:
    case LAYER_ABOUT_CREDITS:
    case LAYER_ABOUT_LEGAL:
      previous_layer = LAYER_ABOUT;
      break;
    case LAYER_SETTINGS_DISPLAY:
    case LAYER_SETTINGS_SOUND:
    case LAYER_SETTINGS_SYSTEM:
      previous_layer = LAYER_SETTINGS;
      break;
    /* WiFi applications */
    case LAYER_WIFI_ANALIZER:
      previous_layer = LAYER_WIFI_APPS;
      break;
    /* Bluetooth applications */
    case LAYER_BLUETOOTH_AIRTAGS_SCAN:
      previous_layer = LAYER_BLUETOOTH_APPS;
      break;
    /* GPS applications */
    case LAYER_GPS_DATE_TIME:
    case LAYER_GPS_LOCATION:
      previous_layer = LAYER_GPS;
      break;
    default:
      ESP_LOGE(TAG_MENU_SCREEN_MODULE, "Invalid layer");
      break;
  }
}

void screen_module_enter_submenu() {
  switch (current_layer) {
    case LAYER_MAIN_MENU: {
      switch (selected_item) {
        case MAIN_MENU_APPLICATIONS:
          current_layer = LAYER_APPLICATIONS;
          break;
        case MAIN_MENU_SETTINGS:
          current_layer = LAYER_SETTINGS;
          break;
        case MAIN_MENU_ABOUT:
          current_layer = LAYER_ABOUT;
          break;
      }
      break;
    }
    case LAYER_APPLICATIONS: {
      switch (selected_item) {
        case APPLICATIONS_MENU_WIFI:
          current_layer = LAYER_WIFI_APPS;
          break;
        case APPLICATIONS_MENU_BLUETOOTH:
          current_layer = LAYER_BLUETOOTH_APPS;
          break;
        case APPLICATIONS_MENU_ZIGBEE:
        case APPLICATIONS_MENU_THREAD:
        case APPLICATIONS_MENU_MATTER:
        case APPLICATIONS_MENU_GPS:
        default:
          break;
      }
      break;
    }
    case LAYER_SETTINGS:
    case LAYER_ABOUT:
    case LAYER_WIFI_APPS:
    case LAYER_BLUETOOTH_APPS:
      switch (selected_item) {
        case BLUETOOTH_MENU_AIRTAGS_SCAN:
          break;
        case BLUETOOTH_MENU_GAME:
          game_engine_state_machine_init();
          break;
        default:
          break;
      }
    case LAYER_ZIGBEE_APPS:
    case LAYER_THREAD_APPS:
    case LAYER_MATTER_APPS:
    case LAYER_WIFI_ANALIZER:
      break;
    default:
      ESP_LOGE(TAG_MENU_SCREEN_MODULE, "Invalid layer");
      break;
  }

  previous_layer = current_layer;
  if (!app_state.in_app) {
    screen_module_display_menu();
  }
}

void screen_module_exit_submenu() {
  ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Exiting submenu");
  ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Previous layer: %d", previous_layer);
  current_layer = previous_layer;
  selected_item = 0;
  screen_module_display_menu();
}

void module_keyboard_update_state(
    bool in_app,
    void (*app_handler)(button_event_t button_pressed)) {
  app_state.in_app = in_app;
  app_state.app_handler = app_handler;
}

app_state_t screen_module_get_app_state() {
  return app_state;
}
