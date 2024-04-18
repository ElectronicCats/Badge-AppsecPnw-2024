#include "modules/cross_keyboard_module.h"
#include "modules/menu_screens_modules.h"

#include "esp_log.h"

esp_err_t button_error;
app_state_t local_keyboard_app_state;

void module_keyboard_button_event_cb(void* arg, void* data) {
  uint8_t button_name =
      (((button_event_t) data) >> 4);  // >> 4 to get the button number
  uint8_t button_event =
      ((button_event_t) data) &
      0x0F;  // & 0x0F to get the event number without the mask
  const char* button_name_str = button_names[button_name];
  const char* button_event_str = button_events_name[button_event];
  if (button_event != BUTTON_PRESS_DOWN) {
    return;
  }

  // If we have a app with a custom handler, we call it
  local_keyboard_app_state = screen_module_get_app_state();
  if (local_keyboard_app_state.in_app) {
    local_keyboard_app_state.app_handler(((button_event_t) data));
    return;
  }

  ESP_LOGI(TAG_CROSS_KEYBOARD_MODULE, "Button: %s, Event: %s", button_name_str,
           button_event_str);

  switch (button_name) {
    case BUTTON_LEFT:
      screen_module_exit_submenu();
      break;
    case BUTTON_RIGHT:
      screen_module_enter_submenu();
      break;
    case BUTTON_UP:
      screen_module_decrement_index_item();
      screen_module_display_menu();
      break;
    case BUTTON_DOWN:
      screen_module_increment_index_item();
      screen_module_display_menu();
      break;
    case BUTTON_BOOT:
    default:
      break;
  }
  screen_module_update_previous_layer();
}

void keyboard_driver_button_register_event_cb(button_handle_t button_handle,
                                              uint8_t mask) {
  ESP_LOGI(TAG_CROSS_KEYBOARD_MODULE,
           "Registering button event callback for button %d", mask >> 4);
  button_error = iot_button_register_cb(button_handle, BUTTON_PRESS_DOWN,
                                        module_keyboard_button_event_cb,
                                        (void*) (BUTTON_PRESS_DOWN | mask));
  button_error |= iot_button_register_cb(button_handle, BUTTON_PRESS_UP,
                                         module_keyboard_button_event_cb,
                                         (void*) (BUTTON_PRESS_UP | mask));
  button_error |= iot_button_register_cb(button_handle, BUTTON_PRESS_REPEAT,
                                         module_keyboard_button_event_cb,
                                         (void*) (BUTTON_PRESS_REPEAT | mask));
  button_error |= iot_button_register_cb(
      button_handle, BUTTON_PRESS_REPEAT_DONE, module_keyboard_button_event_cb,
      (void*) (BUTTON_PRESS_REPEAT_DONE | mask));
  button_error |= iot_button_register_cb(button_handle, BUTTON_SINGLE_CLICK,
                                         module_keyboard_button_event_cb,
                                         (void*) (BUTTON_SINGLE_CLICK | mask));
  button_error |= iot_button_register_cb(button_handle, BUTTON_DOUBLE_CLICK,
                                         module_keyboard_button_event_cb,
                                         (void*) (BUTTON_DOUBLE_CLICK | mask));
  button_error |= iot_button_register_cb(
      button_handle, BUTTON_LONG_PRESS_START, module_keyboard_button_event_cb,
      (void*) (BUTTON_LONG_PRESS_START | mask));
  button_error |= iot_button_register_cb(
      button_handle, BUTTON_LONG_PRESS_HOLD, module_keyboard_button_event_cb,
      (void*) (BUTTON_LONG_PRESS_HOLD | mask));
  button_error |= iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_UP,
                                         module_keyboard_button_event_cb,
                                         (void*) (BUTTON_LONG_PRESS_UP | mask));
  ESP_ERROR_CHECK(button_error);
}

void keyboard_driver_button_init(uint32_t button_pin, uint8_t mask) {
  button_config_t button_config = {
      .type = BUTTON_TYPE_GPIO,
      .gpio_button_config =
          {
              .gpio_num = button_pin,
              .active_level = BUTTON_ACTIVE_LEVEL,
          },
  };

  button_handle_t button = iot_button_create(&button_config);
  assert(button);
  keyboard_driver_button_register_event_cb(button, mask);
}

void module_keyboard_begin() {
  keyboard_driver_button_init(BOOT_BUTTON_PIN, BOOT_BUTTON_MASK);
  keyboard_driver_button_init(LEFT_BUTTON_PIN, LEFT_BUTTON_MASK);
  keyboard_driver_button_init(RIGHT_BUTTON_PIN, RIGHT_BUTTON_MASK);
  keyboard_driver_button_init(UP_BUTTON_PIN, UP_BUTTON_MASK);
  keyboard_driver_button_init(DOWN_BUTTON_PIN, DOWN_BUTTON_MASK);
}
