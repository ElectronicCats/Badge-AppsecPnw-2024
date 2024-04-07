#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "iot_button.h"

#define TAG_KEYBOARD "EC_APPSECPWN_KEYBOARD"
#define BOOT_BUTTON_PIN GPIO_NUM_9
#define LEFT_BUTTON_PIN GPIO_NUM_22
#define RIGHT_BUTTON_PIN GPIO_NUM_1
#define UP_BUTTON_PIN GPIO_NUM_15
#define DOWN_BUTTON_PIN GPIO_NUM_23
#define BUTTON_ACTIVE_LEVEL 0

#define BOOT_BUTTON_MASK 0b0000 << 4
#define LEFT_BUTTON_MASK 0b0001 << 4
#define RIGHT_BUTTON_MASK 0b0010 << 4
#define UP_BUTTON_MASK 0b0011 << 4
#define DOWN_BUTTON_MASK 0b0100 << 4

typedef enum {
    BUTTON_BOOT = 0,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_UP,
    BUTTON_DOWN,
} ButtonType;

typedef struct {
    bool in_app;
    void (*app_handler)(ButtonType button_pressed);
} KeyboardState;


static KeyboardState keyboard_state = {
    .in_app = false,
    .app_handler = NULL,
};

static void button_event_cb(void* arg, void* data);
void button_init(uint32_t button_num, uint8_t mask);
void keyboard_init();
void handle_back();
void handle_selected_option();
void update_previous_layer();
void handle_main_selection();
void handle_applications_selection();
void handle_settings_selection();
void handle_about_selection();
void handle_wifi_apps_selection();
void handle_bluetooth_apps_selection();
void handle_thread_apps_selection();
void handle_keyboard_events(ButtonType button);
#endif // KEYBOARD_H