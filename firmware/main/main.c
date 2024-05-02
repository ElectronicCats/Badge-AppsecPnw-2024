/*
* Electronic Cats AppsecPwn CTF 2024

* This file is part of the Electronic Cats AppsecPwn CTF 2024 project.
*/
#include "captive_portal.h"
#include "esp_event.h"
#include "modules/cross_keyboard_module.h"
#include "modules/game_screens_module.h"
#include "modules/menu_screens_modules.h"
#include "nvs_memory.h"

void app_main(void) {
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  memory_driver_begin();
  module_keyboard_begin();
  screen_module_begin();
  screen_module_set_main_menu();
  screen_module_display_badge_logo();
  captive_portal_begin();
}
