/*
* Electronic Cats AppsecPwn CTF 2024

* This file is part of the Electronic Cats AppsecPwn CTF 2024 project.
*/
#include "esp_event.h"
#include "esp_log.h"
#include "modules/cross_keyboard_module.h"
#include "modules/game_screens_module.h"
#include "modules/menu_screens_modules.h"
#include "preferences.h"
#include "modules/led_events.h"

const char* TAG_MAIN = "MAIN";

void app_main(void) {
  // esp_log_set_level_master(ESP_LOG_INFO);
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_err_t err = preferences_begin();
  if (err != ESP_OK) {
    ESP_LOGE(TAG_MAIN, "Error initializing preferences: %s",
             esp_err_to_name(err));
  }
  // preferences_clear();
  module_keyboard_begin();
  screen_module_begin();
  int last_layer = preferences_get_int("MENUNUMBER", 99);
  if (last_layer == 99) {
    screen_module_set_main_menu();
    screen_module_display_badge_logo();
  } else {
    screen_module_set_screen(last_layer);
    screen_module_display_menu();
    preferences_put_int("MENUNUMBER", 99);
  }
}
