#include "modules/menu_screens_modules.h"
#include <stdlib.h>
#include <string.h>
#include "drivers/oled_ssd1306_driver.h"
#include "esp_log.h"
#include "modules/ble/ble_module.h"
#include "modules/device/device_screens_module.h"
#include "modules/game_engine_module.h"
#include "modules/wifi/wifi_module.h"
#include "modules/zigbee/zigbee_module.h"
#include "preferences.h"

static screen_module_layer_t previous_layer;
static screen_module_layer_t current_layer;
static uint8_t selected_item;
static int submenu_items = 0;

static app_state_t app_state = {
    .in_app = false,
    .app_handler = NULL,
};

static void settings_module_state_machine(button_event_t button_pressed);
static void screen_module_display_settings_display();

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

int screen_module_get_count_menu_items(char* items[]) {
  int count = 0;
  while (items[count] != NULL) {
    count++;
  }
  return count;
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

  ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Submenu items: %d", submenu_items);

  return screen_module_add_empty_strings(menu_items[current_layer],
                                         submenu_items);
}

void screen_module_display_menu_items(char** items) {
  // Show only 3 options at a time in the following order:
  // Page 1: Option 1
  // Page 3: Option 2 -> selected option
  // Page 5: Option 3
  int page = 0;
  char* terminal_user = "appsec@pwn";
  oled_driver_display_text(page, terminal_user, OLED_DISPLAY_NORMAL);
  page++;
  int items_list = submenu_items - 2;

  for (int i = 0; i < items_list; i++) {
    char* name = (char*) malloc(20);
    ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Item: %s", items[i]);

    strcpy(name, items[i + 1]);
    if (selected_item == i) {
      char* prefix = "> ";
      char item_text[strlen(prefix) + strlen(name) + 1];
      strcpy(item_text, prefix);
      strcat(item_text, name);
      oled_driver_display_text(page, item_text, OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_text(page, name, OLED_DISPLAY_NORMAL);
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
  preferences_put_int("MENUNUMBER", 99);
}

void screen_module_set_screen(int screen_layer) {
  previous_layer = screen_layer;
  current_layer = screen_layer;
  submenu_items = 0;
  preferences_put_int("MENUNUMBER", screen_layer);
  screen_module_display_menu();
}

void screen_module_display_in_progress() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(4, "In Progress", OLED_DISPLAY_NORMAL);
}

void screen_module_increment_index_item() {
  if (submenu_items == 0) {
    return;
  }

  ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Selected item: %d - %d", selected_item,
           submenu_items);

  if (selected_item < submenu_items - 3) {
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
    selected_item = submenu_items - 3;
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
      // case LAYER_MATTER_APPS:
      // case LAYER_GPS:
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
    case LAYER_WIFI_DEAUTH:
      previous_layer = LAYER_WIFI_APPS;
      break;
    /* Bluetooth applications */
    case LAYER_BLUETOOTH_AIRTAGS_SCAN:
    case LAYER_BLUETOOTH_SPAM:
    case LAYER_BLUETOOTH_GAME:
      previous_layer = LAYER_BLUETOOTH_APPS;
      break;
    case LAYER_ZIGBEE_SNIFFER:
      previous_layer = LAYER_ZIGBEE_APPS;
      break;
    /* GPS applications */
    // case LAYER_GPS_DATE_TIME:
    // case LAYER_GPS_LOCATION:
    //   previous_layer = LAYER_GPS;
    //   break;
    default:
      ESP_LOGE(TAG_MENU_SCREEN_MODULE, "Invalid layer");
      break;
  }
}

void screen_module_enter_submenu() {
  ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Last submenu: Layer %d - Selected: %d",
           current_layer, selected_item);
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
        default:
          current_layer = LAYER_MAIN_MENU;
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
          current_layer = LAYER_ZIGBEE_APPS;
          break;
        case APPLICATIONS_MENU_THREAD:
        case APPLICATIONS_MENU_MATTER:
        case APPLICATIONS_MENU_GPS:
        default:
          current_layer = LAYER_APPLICATIONS;
          break;
      }
      break;
    }
    case LAYER_WIFI_APPS: {
      switch (selected_item) {
        // case WIFI_MENU_ANALIZER:
        //  current_layer = LAYER_WIFI_ANALIZER;
        //  screen_module_display_in_progress();
        //  break;
        case WIFI_MENU_DEAUTH:
          current_layer = LAYER_WIFI_DEAUTH;
          wifi_module_begin();
          break;
        default:
          break;
      }
      break;
    }
    case LAYER_BLUETOOTH_APPS: {
      switch (selected_item) {
        case BLUETOOTH_MENU_AIRTAGS_SCAN:
          current_layer = LAYER_BLUETOOTH_AIRTAGS_SCAN;
          ble_module_begin(selected_item);
          break;
        case BLUETOOTH_MENU_SPAM:
          current_layer = LAYER_BLUETOOTH_SPAM;
          ble_module_begin(selected_item);
          break;
        case BLUETOOTH_MENU_GAME:
          current_layer = LAYER_BLUETOOTH_GAME;
          game_engine_state_machine_init();
          break;
        default:
          break;
      }
      break;
    }
    case LAYER_SETTINGS:
      switch (selected_item) {
        case SETTINGS_MENU_DISPLAY:
          current_layer = LAYER_SETTINGS_DISPLAY;
          selected_item = 0;
          module_keyboard_update_state(true, settings_module_state_machine);
          screen_module_display_settings_display();
          break;
        default:
          break;
      }
      break;
    case LAYER_ABOUT:
      switch (selected_item) {
        case ABOUT_MENU_VERSION:
          current_layer = LAYER_ABOUT_VERSION;
          device_screens_module_version();
          break;
        case ABOUT_MENU_LICENSE:
          current_layer = LAYER_ABOUT_LICENSE;
          device_screens_module_licence();
          break;
        case ABOUT_MENU_CREDITS:
          current_layer = LAYER_ABOUT_CREDITS;
          device_screens_module_credits();
          break;
        case ABOUT_MENU_LEGAL:
          current_layer = LAYER_ABOUT_LEGAL;
          device_screens_module_legal();
          break;
        default:
          break;
      }
      break;
    case LAYER_ZIGBEE_APPS:
      switch (selected_item) {
        case ZIGBEE_MENU_SNIFFER:
          current_layer = LAYER_ZIGBEE_SNIFFER;
          zigbee_module_begin(selected_item);
          break;
        default:
          break;
      }
      // case LAYER_MATTER_APPS:
      break;
    default:
      ESP_LOGE(TAG_MENU_SCREEN_MODULE, "Invalid layer");
      break;
  }

  previous_layer = current_layer;
  selected_item = 0;
  if (!app_state.in_app) {
    screen_module_display_menu();
  }
}

void screen_module_exit_submenu() {
  ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Exiting submenu");
  ESP_LOGI(TAG_MENU_SCREEN_MODULE, "Previous layer: %d Current: %d",
           previous_layer, current_layer);
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

screen_module_layer_t screen_module_get_current_layer() {
  return current_layer;
}

static void settings_module_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;

  ESP_LOGI(TAG_BLE_MODULE, "BLE engine state machine from team: %d %d",
           button_name, button_event);
  switch (button_name) {
    case BUTTON_LEFT:
      module_keyboard_update_state(false, NULL);
      screen_module_exit_submenu();
      break;
    case BUTTON_RIGHT:
      int is_jedi_unlocked = preferences_get_int("UBADGEJEDI", 99);
      int is_sith_unlocked = preferences_get_int("UBADGESITH", 99);
      if ((selected_item == 1 && is_jedi_unlocked == 0) ||
          (selected_item == 2 && is_sith_unlocked == 0)) {
        oled_driver_clear(OLED_DISPLAY_NORMAL);
        oled_driver_display_text_center(3, "Jedi badge locked",
                                        OLED_DISPLAY_NORMAL);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        module_keyboard_update_state(false, NULL);
        screen_module_exit_submenu();
        break;
      }
      preferences_put_int("SHOWBADGE", selected_item);
      oled_driver_clear(OLED_DISPLAY_NORMAL);
      oled_driver_display_text_center(3, "Splash changed", OLED_DISPLAY_NORMAL);
      oled_driver_display_text_center(4, "Rebooting card", OLED_DISPLAY_NORMAL);
      oled_driver_display_text_center(5, "To apply changes",
                                      OLED_DISPLAY_NORMAL);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      module_keyboard_update_state(false, NULL);
      screen_module_exit_submenu();
      break;
    case BUTTON_UP:
      selected_item = (selected_item == 0) ? 2 : selected_item - 1;
      screen_module_display_settings_display();
      break;
    case BUTTON_DOWN:
      selected_item = (selected_item == 2) ? 0 : selected_item + 1;
      screen_module_display_settings_display();
      break;
    case BUTTON_BOOT:
    default:
      break;
  }
}

static void screen_module_display_settings_display() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "Display", OLED_DISPLAY_NORMAL);
  int is_jedi_unlocked = preferences_get_int("UBADGEJEDI", 99);
  int is_sith_unlocked = preferences_get_int("UBADGESITH", 99);
  static char* background_splash[] = {
      "Default",
      "Jedi",
      "Sith",
      NULL,
  };

  for (int i = 0; i < 3; i++) {
    if (selected_item == i) {
      if (strcmp(background_splash[i], "Jedi") == 0 && is_jedi_unlocked == 0) {
        char item_text[strlen(background_splash[i]) + strlen("Locked") + 3];
        sprintf(item_text, "> %s Locked", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_INVERTED);
      } else if (strcmp(background_splash[i], "Jedi") == 0 &&
                 is_jedi_unlocked == 1) {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "> %s", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_INVERTED);
      } else if (strcmp(background_splash[i], "Sith") == 0 &&
                 is_sith_unlocked == 0) {
        char item_text[strlen(background_splash[i]) + strlen("Locked") + 3];
        sprintf(item_text, "> %s Locked", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_INVERTED);
      } else if (strcmp(background_splash[i], "Sith") == 0 &&
                 is_sith_unlocked == 1) {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "> %s", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_INVERTED);
      } else {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "> %s", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_INVERTED);
      }
    } else {
      if (strcmp(background_splash[i], "Jedi") == 0 && is_jedi_unlocked == 0) {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "%s Locked", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_NORMAL);
      } else if (strcmp(background_splash[i], "Jedi") == 0 &&
                 is_jedi_unlocked == 1) {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "%s", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_NORMAL);
      } else if (strcmp(background_splash[i], "Sith") == 0 &&
                 is_sith_unlocked == 0) {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "%s Locked", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_NORMAL);
      } else if (strcmp(background_splash[i], "Sith") == 0 &&
                 is_sith_unlocked == 1) {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "%s", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_NORMAL);
      } else {
        char item_text[strlen(background_splash[i]) + 3];
        sprintf(item_text, "%s", background_splash[i]);
        oled_driver_display_text(i + 2, item_text, OLED_DISPLAY_NORMAL);
      }
    }
  }
}
