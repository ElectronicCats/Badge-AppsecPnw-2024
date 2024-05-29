#include "modules/led_events.h"
#include "freertos/FreeRTOS.h"
#include "rgb_ledc_controller.h"
#include "ledc_controller.h"

static TaskHandle_t ble_led_evenet_task = NULL;
static rgb_led_t ble_led_controller;
static bool led_control = false;
led_t led_red, led_green, led_blue;

void led_control_begin(void) {
  // ble_led_controller = rgb_led_new(GPIO_LED_RED, GPIO_LED_GREEN, GPIO_LED_BLUE, LEDC_CHANNEL_0,
  //                 LEDC_CHANNEL_1, LEDC_CHANNEL_2);
  // ESP_ERROR_CHECK(rgb_led_init(&ble_led_controller));
  led_control = true;

  led_red = led_controller_led_new(GPIO_LED_RED, LEDC_CHANNEL_0);
  led_green = led_controller_led_new(GPIO_LED_GREEN, LEDC_CHANNEL_1);
  led_blue = led_controller_led_new(GPIO_LED_BLUE, LEDC_CHANNEL_2);
  led_controller_led_init(&led_red);
  led_controller_led_init(&led_green);
  led_controller_led_init(&led_blue);
  // led_controller_led_on(&led_blue);
  // led_controller_led_on(&led_red);
  // led_controller_led_on(&led_green);

}

void led_control_game_event_pairing(void) {
  // while (led_control) {
  //   rgb_led_start_breath_effect(&ble_led_controller, BLUE, 100);
  //   vTaskDelay(pdMS_TO_TICKS(6000));
  // }
  // rgb_led_set_color(&ble_led_controller, BLUE);
  led_controller_led_on(&led_red);
  led_controller_led_on(&led_green);
  led_controller_led_off(&led_blue);
  //rgb_led_start_breath_effect(&ble_led_controller, BLUE, 100);
  vTaskSuspend(NULL);
  // rgb_led_deinit(&ble_led_controller);
}

void led_control_game_event_blue_team_turn(void) {
  led_controller_led_on(&led_red);
  led_controller_led_on(&led_green);
  led_controller_led_off(&led_blue);
  //led_controller_start_breath_effect(&led_blue, 20);
  //rgb_led_start_blink_effect(&ble_led_controller, BLUE, 3, 800, 800, 2000);
  vTaskSuspend(NULL);
  // rgb_led_deinit(&ble_led_controller);
}

void led_control_game_event_red_team_turn(void) {
  led_controller_led_on(&led_blue);
  led_controller_led_on(&led_green);
  led_controller_led_off(&led_red);
  //led_controller_start_blink_effect(&led_red, 3, 100, 100, 100, 100);
  vTaskSuspend(NULL);
  // rgb_led_deinit(&ble_led_controller);
}

void led_control_game_event_attacking(void) {
  led_controller_led_off(&led_red);
  led_controller_led_off(&led_green);
  led_controller_led_on(&led_blue);
  // rgb_led_stop_any_effect(&ble_led_controller);
   vTaskSuspend(NULL);
  //  rgb_led_deinit(&ble_led_controller);
}

void led_control_game_event_red_team_winner(void) {
  led_controller_led_on(&led_blue);
  led_controller_led_on(&led_green);
  led_controller_led_off(&led_red);
  // rgb_led_stop_any_effect(&ble_led_controller);
   vTaskSuspend(NULL);
  //  rgb_led_deinit(&ble_led_controller);
}

void led_control_game_event_blue_team_winner(void) {
  led_controller_led_off(&led_blue);
  led_controller_led_on(&led_green);
  led_controller_led_on(&led_red);
  // rgb_led_stop_any_effect(&ble_led_controller);
   vTaskSuspend(NULL);
  //  rgb_led_deinit(&ble_led_controller);
}

void led_control_ble_tracking(void) {
  led_controller_led_off(&led_blue);
  led_controller_led_on(&led_green);
  led_controller_led_on(&led_red);
   vTaskSuspend(NULL);
  //  rgb_led_deinit(&ble_led_controller);
}

void led_control_ble_spam_breathing(void) {
  led_controller_led_off(&led_blue);
  led_controller_led_on(&led_green);
  led_controller_led_off(&led_red);
  vTaskSuspend(NULL);
  // rgb_led_deinit(&ble_led_controller);
}

void led_control_wifi_scanning(void) {
  led_controller_led_off(&led_blue);
  led_controller_led_off(&led_green);
  led_controller_led_on(&led_red);
  // rgb_led_stop_any_effect(&ble_led_controller);
   vTaskSuspend(NULL);
  //  rgb_led_deinit(&ble_led_controller);
}

void led_control_wifi_attacking(void) {
  led_controller_led_off(&led_blue);
  led_controller_led_on(&led_green);
  led_controller_led_off(&led_red);
  // rgb_led_stop_any_effect(&ble_led_controller);
   vTaskSuspend(NULL);
  //  rgb_led_deinit(&ble_led_controller);
}

void led_control_zigbee_scanning(void) {
  led_controller_led_off(&led_blue);
  led_controller_led_off(&led_green);
  led_controller_led_on(&led_red);
  // rgb_led_stop_any_effect(&ble_led_controller);
   vTaskSuspend(NULL);
  //  rgb_led_deinit(&ble_led_controller);
}

void led_control_stop(void) {
  if (led_control) {
    led_control = false;
    rgb_led_stop_any_effect(&ble_led_controller);

    rgb_led_start_blink_effect(&ble_led_controller, 0x000000, 3, 100, 200, 400);
  }
}

void led_control_run_effect(effect_coontrol effect_function) {
  if (ble_led_evenet_task != NULL) {
    vTaskDelete(ble_led_evenet_task);
    // rgb_led_deinit(&ble_led_controller);
  }

  if (!led_control) {
    led_control_begin();
  }

  xTaskCreate(effect_function, "effect_function", 8192, NULL, 20,
              &ble_led_evenet_task);
}
