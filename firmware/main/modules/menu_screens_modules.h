#include "drivers/oled_ssd1306_driver.h"
#include "modules/cross_keyboard_module.h"
#include "modules/screens_modules.h"
#ifndef MENU_SCREENS_MODULE_H
  #define MENU_SCREENS_MODULE_H
  #define TAG_MENU_SCREEN_MODULE    "MENU_SCREEN_MODULE"
  #define MAX_MENU_ITEMS_PER_SCREEN 3

/**
 * @brief Structure to store the app screen state information
 *
 */
typedef struct {
  bool in_app;
  int app_selected;
} app_screen_state_information_t;

typedef enum {
  SCREEN_IN_NAVIGATION = 0,
  SCREEN_IN_APP,
} screen_module_controls_type_t;

/**
 * @brief Initialize the screen module
 */
void screen_module_begin(void);

/**
 * @brief Add empty strings to the screen
 *
 * @param char** array
 * @param int length
 * @return char**
 */
char** screen_module_add_empty_strings(char** array, int length);

/**
 * @brief Get the menu items
 *
 * @return char** Items of the menu
 */
char** screen_module_get_menu_items(void);

/**
 * @brief Display the menu items
 *
 * @param char** items
 * @return void
 */
void screen_module_display_menu_items(char** items);

/**
 * @brief Display the menu
 *
 * @return void
 */
void screen_module_display_menu(void);

/**
 * @brief Set the main menu
 *
 * @return void
 */
void screen_module_set_main_menu(void);

/**
 * @brief Increment the index item
 *
 * @return void
 */
void screen_module_increment_index_item(void);

/**
 * @brief Decrement the index item
 *
 * @return void
 */
void screen_module_decrement_index_item(void);

/**
 * @brief Display the in progress screen
 *
 * @return void
 */
void screen_module_display_in_progress(void);

/**
 * @brief Update the previous layer
 *
 * @return void
 */
void screen_module_update_previous_layer(void);

/**
 * @brief Enter the submenu
 *
 * @return void
 */
void screen_module_enter_submenu(void);

/**
 * @brief Exit the submenu
 *
 * @return void
 */
void screen_module_exit_submenu(void);

/**
 * @brief Struct to hold the keyboard state in app
 */
typedef struct {
  bool in_app;
  void (*app_handler)(button_event_t button_pressed);
} app_state_t;

/**
 * @brief Update the keyboard state
 *
 * @param bool in_app
 * @param void app_handler function pointer
 */
void module_keyboard_update_state(
    bool in_app,
    void (*app_handler)(button_event_t button_pressed));

/**
 * @brief Get the app state
 *
 * @return app_state_t
 */
app_state_t screen_module_get_app_state(void);

/**
 * @brief Get the current layer
 *
 * @return screen_module_layer_t
 */
screen_module_layer_t screen_module_get_current_layer(void);
#endif  // MENU_SCREENS_MODULE_H
