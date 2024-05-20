#include "iot_button.h"
#ifndef CROSS_KEYBOARD_MODULE_H
  #define CROSS_KEYBOARD_MODULE_H

  #define TAG_CROSS_KEYBOARD_MODULE "KEYBOARD_MODULE"
  #define BOOT_BUTTON_PIN           CONFIG_BOOT_BUTTON
  #define LEFT_BUTTON_PIN           CONFIG_GPIO_LEFT_BUTTON
  #define RIGHT_BUTTON_PIN          CONFIG_GPIO_RIGHT_BUTTON
  #define UP_BUTTON_PIN             CONFIG_GPIO_UP_BUTTON
  #define DOWN_BUTTON_PIN           CONFIG_GPIO_DOWN_BUTTON

  #define BOOT_BUTTON_MASK  0b0000 << 4
  #define LEFT_BUTTON_MASK  0b0001 << 4
  #define RIGHT_BUTTON_MASK 0b0010 << 4
  #define UP_BUTTON_MASK    0b0011 << 4
  #define DOWN_BUTTON_MASK  0b0100 << 4

  #define BUTTON_ACTIVE_LEVEL 0
/**
 * @brief Events for the available keyboard buttons events
 *
 */
static const char* button_events_name[] = {
    "BUTTON_PRESS_DOWN",      "BUTTON_PRESS_UP",
    "BUTTON_PRESS_REPEAT",    "BUTTON_PRESS_REPEAT_DONE",
    "BUTTON_SINGLE_CLICK",    "BUTTON_DOUBLE_CLICK",
    "BUTTON_MULTIPLE_CLICK",  "BUTTON_LONG_PRESS_START",
    "BUTTON_LONG_PRESS_HOLD", "BUTTON_LONG_PRESS_UP",
};

/**
 * @brief Layout of the available keyboard buttons names
 *
 */
static const char* button_names[] = {
    "BOOT", "LEFT", "RIGHT", "UP", "DOWN",
};

/**
 * @brief Enum of the available keyboard buttons
 *
 */
typedef enum {
  BUTTON_BOOT = 0,
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_UP,
  BUTTON_DOWN,
} keyboard_buttons_layout_t;

/**
 * @brief Struct to hold the button state
 *
 */
typedef struct {
  uint8_t button_pressed;
  uint8_t button_event;
} button_event_state_t;

/**
 * @brief Initialize the keyboard button
 *
 * @param uint32_t button_pin Button pin
 * @param uint8_t mask Mask
 * @param void handler function pointer for the button event callback
 *
 * @return void
 */
void keyboard_driver_button_init(uint32_t button_pin, uint8_t mask);

/**
 * @brief Register the event callback for the keyboard button
 *
 * @param button_handle_t button_handle
 * @param uint8_t mask
 *
 * @return void
 */
void keyboard_driver_button_register_event_cb(button_handle_t button_handle,
                                              uint8_t mask);

/**
 * @brief Keyboard button event callback
 *
 * @param void* arg
 * @param void* data
 *
 * @return void
 */
void module_keyboard_button_event_cb(void* arg, void* data);

/**
 * @brief Initialize the keyboard module
 *
 * @return void
 */
void module_keyboard_begin();

#endif  // CROSS_KEYBOARD_MODULE_H
