#include "modules/ble/airtags_module.h"
#include "modules/cross_keyboard_module.h"
#include "modules/menu_screens_modules.h"
#ifndef APP_SCREEN_MODULE_H
  #define APP_SCREEN_MODULE_H

  #define TAG_APP_SCREEN_MODULE "APP_SCREEN_MODULE"

typedef struct {
  bool in_app;
  int app_selected;
} app_screen_state_information_t;

void app_screen_state_machine_init(int app_selected);
void app_screen_select_app(void);
void app_screen_state_machine(button_event_t button_pressed);
void screen_app_display_bluetooth_scanner(tracker_profile_t record);
void screen_app_task_display_devices();
void screen_app_task_start_display_devices();
void screen_app_task_stop_display_devices();
void screen_app_display_modal(tracker_profile_t profile);
#endif  // APP_SCREEN_MODULE_H
