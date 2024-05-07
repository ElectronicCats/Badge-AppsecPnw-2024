#include "modules/game_engine_module.h"
#include "bt_gattc.h"
#include "bt_gatts.h"
#include "modules/cross_keyboard_module.h"
#include "modules/game/game_ble_client.h"
#include "modules/game/game_ble_server.h"
#include "modules/game_owasp_profile.h"
#include "modules/game_screens_module.h"
#include "modules/led_events.h"
#include "modules/menu_screens_modules.h"
#include "preferences.h"

#include <esp_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"

game_match_information_t current_game_state;
uint8_t ble_command[GAME_BLE_COMMAND_LENGTH];
int current_option = 0;
int current_turn = 0;
int last_profile_index = 0;
uint16_t game_profile_history[GAME_OWASP_PROFILES_COUNT];

static void game_engine_int_profile_history();
static int game_engine_find_profile_history(int profile_index);
static void game_engine_set_profile_history();
static void game_engine_decrement_turn();
static void game_engine_handle_battle_winner_profile();
static int game_engine_get_owasp_profile_index(game_owasp_profile_t* profile);
static int generate_random_float();

static int generate_random_float() {
  int random_num = rand() % (1 - 0 + 1) + 0;
  return random_num;
}

static void game_engine_int_profile_history() {
  preferences_clear();
  bool is_configured = preferences_get_bool(PREFERENCE_CONFIGURED_TAG, false);
  for (int i = 0; i < GAME_OWASP_PROFILES_COUNT; i++) {
    game_profile_history[i] = 0;
    if (!is_configured) {
      char* profile_name = malloc(10);
      sprintf(profile_name, "owasp_%d ", i);
      int profile = preferences_put_int(profile_name, 0);
      game_profile_history[i] = profile;
      free(profile_name);
    }
  }

  if (!is_configured) {
    preferences_put_bool(PREFERENCE_CONFIGURED_TAG, true);
    preferences_put_int(PREFERENCE_PROFILE_LEVEL_TAG, 0);
  }
}

static int game_engine_find_profile_history(int profile_index) {
  for (int i = 0; i < GAME_OWASP_PROFILES_COUNT; i++) {
    if (game_profile_history[i] == profile_index) {
      return 1;
    }
  }
  return 0;
}

static void game_engine_decrement_turn() {
  game_profile_history[last_profile_index] = 0;
}

static void game_engine_set_profile_history() {
  int profile_index =
      game_engine_get_owasp_profile_index(current_game_state.attacker_profile);
  game_profile_history[profile_index] = 1;
  last_profile_index = profile_index;
}

static void game_engine_handle_unlock_badge() {
  int profile_level = preferences_get_int(PREFERENCE_PROFILE_LEVEL_TAG, 99);
  if (profile_level >= 3 && profile_level < 6) {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Badge unlocked: 3");
  } else if (profile_level >= 6 && profile_level < 9) {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Badge unlocked: 6");
  } else if (profile_level >= 9 && profile_level < 10) {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Badge unlocked: 9");
  } else if (profile_level >= 10) {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Badge unlocked: 10");
  }
}

static void game_engine_handle_battle_winner_profile() {
  int unlocked_badge = 0;
  uint16_t profiles_list[GAME_OWASP_PROFILES_COUNT];
  // Get the saved profiles
  for (int i = 0; i < GAME_OWASP_PROFILES_COUNT; i++) {
    char* profile_name = malloc(10);
    sprintf(profile_name, "owasp_%d ", i);
    int profile = preferences_get_int(profile_name, 99);
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Profile history owasp: %d", profile);
    profiles_list[i] = profile;
    if (profile == 1) {
      unlocked_badge++;
    }
    free(profile_name);
  }

  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Unlocked badges: %d", unlocked_badge);
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Profile history index|current|prev");
  for (int i = 0; i < GAME_OWASP_PROFILES_COUNT; i++) {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, " Profile %d: %d - %d", i,
             game_profile_history[i], profiles_list[i]);
    char* profile_name = malloc(10);
    sprintf(profile_name, "owasp_%d ", i);
    preferences_put_int(profile_name, game_profile_history[i]);
    free(profile_name);
  }

  preferences_put_int(PREFERENCE_PROFILE_LEVEL_TAG, unlocked_badge);

  for (int i = 0; i < GAME_OWASP_PROFILES_COUNT; i++) {
    char* profile_name = malloc(10);
    sprintf(profile_name, "owasp_%d ", i);
    int profile = preferences_get_int(profile_name, 99);
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Profile history saved owasp: %d",
             profile);
    profiles_list[i] = profile;
    free(profile_name);
  }

  uint32_t profile_level =
      preferences_get_int(PREFERENCE_PROFILE_LEVEL_TAG, 99);
}

void game_engine_state_machine_init() {
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Initializing game engine state machine");
  module_keyboard_update_state(true, game_engine_state_machine);

  current_game_state.game_state = GAME_STATE_TEAM_SELECTION;
  screen_module_display_game_screen();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  game_engine_display_team_selection();
  game_engine_int_profile_history();
};

void game_engine_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;  // >> 4 to get the button number
  uint8_t button_event =
      button_pressed & 0x0F;  // & 0x0F to get the event number without the mask

  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Game engine state machine from team: %d %d",
           button_name, button_event);
  switch (current_game_state.game_state) {
    case GAME_STATE_TEAM_SELECTION:
      switch (button_name) {
        case BUTTON_LEFT:
          if (button_event == BUTTON_DOUBLE_CLICK) {
            module_keyboard_update_state(false, NULL);
            screen_module_exit_submenu();
            break;
          }

          break;
        case BUTTON_RIGHT:
          current_game_state.team =
              (current_option == 0) ? GAME_TEAM_BLUE : GAME_TEAM_RED;
          current_game_state.game_state = GAME_STATE_PAIRING_DEVICES;

          current_game_state.attacker.life_points = GAME_DEFAULT_LIFE_POINTS;
          current_game_state.opponent.life_points = GAME_DEFAULT_LIFE_POINTS;

          led_control_run_effect(led_control_game_event_pairing);

          if (current_game_state.team == GAME_TEAM_BLUE) {
            ble_client_task_begin();
            screen_module_display_game_pairing_client();
          } else {
            ble_server_task_begin();
            screen_module_display_game_pairing_server();
          }
          current_option = 0;
          break;
        case BUTTON_UP:
          current_option = (current_option == 0) ? 0 : current_option - 1;
          game_engine_display_team_selection();
          break;
        case BUTTON_DOWN:
          current_option =
              (current_option == 1) ? current_option : current_option + 1;
          game_engine_display_team_selection();
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    case GAME_STATE_ATTACKER_PROFILE_SELECTION:
      switch (button_name) {
        case BUTTON_LEFT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button left pressed");
          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          game_owasp_profile_t* profile_selected =
              game_engine_get_owasp_profile(current_option);
          current_game_state.attacker_profile = profile_selected;
          current_game_state.opponent_profile = profile_selected;

          current_game_state.game_state = GAME_STATE_ATTACKER_ATTACK_SELECTION;
          ble_command[0] = BLE_COMMAND_PROFILE_SELECTED;
          ble_command[1] = current_option;
          ble_server_send_data(ble_command, GAME_BLE_COMMAND_LENGTH);

          vTaskDelay(1000 / portTICK_PERIOD_MS);
          current_option = 0;
          game_engine_display_owasp_profile_attacks();

          break;
        case BUTTON_UP:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          game_engine_display_owasp_profile_selection();
          break;
        case BUTTON_DOWN:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button down pressed");
          current_option = (current_option == GAME_OWASP_PROFILES_COUNT - 1)
                               ? 0
                               : current_option + 1;
          game_engine_display_owasp_profile_selection();
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    case GAME_STATE_ATTACKER_ATTACK_SELECTION:
      switch (button_name) {
        case BUTTON_LEFT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button left pressed");
          if (current_game_state.team == GAME_TEAM_RED) {
            current_game_state.game_state =
                GAME_STATE_ATTACKER_PROFILE_SELECTION;
            game_engine_display_owasp_profile_selection();
            game_engine_decrement_turn();
          }
          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          current_game_state.game_state =
              GAME_STATE_OPPONENT_RESPONSE_SELECTION;
          current_game_state.attacker_action = current_option;

          led_control_run_effect(led_control_game_event_attacking);

          ble_command[0] = BLE_COMMAND_ATTACK_SELECTED;
          ble_command[1] = current_option;
          ble_server_send_data(ble_command, GAME_BLE_COMMAND_LENGTH);

          game_engine_set_profile_history();

          screen_module_display_game_sending_attack();
          break;
        case BUTTON_UP:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          game_engine_display_owasp_profile_attacks();
          break;
        case BUTTON_DOWN:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button down pressed");
          current_option =
              (current_option == 1) ? current_option : current_option + 1;
          game_engine_display_owasp_profile_attacks();
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    case GAME_STATE_OPPONENT_RESPONSE_SELECTION:
      switch (button_name) {
        case BUTTON_LEFT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button left pressed");
          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          current_game_state.opponent_action = current_option;
          ble_command[0] = BLE_COMMAND_RESPONSE_SELECTED;
          ble_command[1] = current_option;
          ble_client_send_data(ble_command, GAME_BLE_COMMAND_LENGTH);

          game_engine_handle_battle_round_winner();
          break;
        case BUTTON_UP:
          if (current_game_state.team == GAME_TEAM_RED) {
            break;
          }
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          game_engine_display_owasp_profile_response();
          break;
        case BUTTON_DOWN:
          if (current_game_state.team == GAME_TEAM_RED) {
            break;
          }
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button down pressed");
          current_option =
              (current_option == 1) ? current_option : current_option + 1;
          game_engine_display_owasp_profile_response();
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    case GAME_STATE_PAIRING_DEVICES:
    default:
      break;
  }
};

void game_engine_handle_game_over() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  if (current_game_state.attacker.life_points <= 0) {
    screen_module_display_game_red_team_logo();
    led_control_run_effect(led_control_game_event_red_team_winner);
  } else {
    led_control_run_effect(led_control_game_event_blue_team_winner);
    screen_module_display_game_blue_team_logo();
  }
  game_engine_handle_battle_winner_profile();
  oled_driver_display_text_center(7, "WINNER", OLED_DISPLAY_NORMAL);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  oled_driver_display_fadeout();
  esp_restart();
}

void game_engine_handle_battle_round_winner() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  if (current_game_state.opponent_action ==
      current_game_state.attacker_action) {
    current_game_state.opponent.life_points -= GAME_DEFAULT_ATTACK_POINTS;
    screen_module_display_game_blue_team_logo();
    led_control_run_effect(led_control_game_event_blue_team_turn);
    oled_driver_display_text_center(7, "DEFENDED", OLED_DISPLAY_NORMAL);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    oled_driver_display_text(0, "Attacker", OLED_DISPLAY_NORMAL);
    screen_module_display_game_points_life(
        current_game_state.opponent.life_points);
    if (current_game_state.opponent.life_points <= 0) {
      game_engine_handle_game_over();
      return;
    }

  } else {
    current_game_state.attacker.life_points -= GAME_DEFAULT_ATTACK_POINTS;
    screen_module_display_game_red_team_logo();
    led_control_run_effect(led_control_game_event_red_team_turn);
    oled_driver_display_text_center(7, "PWNED", OLED_DISPLAY_NORMAL);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    oled_driver_display_text(0, "Defender", OLED_DISPLAY_NORMAL);
    screen_module_display_game_points_life(
        current_game_state.attacker.life_points);
    if (current_game_state.attacker.life_points <= 0) {
      game_engine_handle_game_over();
      return;
    }
  }

  vTaskDelay(2000 / portTICK_PERIOD_MS);

  if (current_game_state.team == GAME_TEAM_BLUE) {
    current_game_state.game_state = GAME_STATE_ATTACKER_PROFILE_SELECTION;
    screen_module_display_game_waiting_profile();
    led_control_run_effect(led_control_game_event_red_team_turn);
  } else {
    current_game_state.game_state = GAME_STATE_ATTACKER_PROFILE_SELECTION;
    game_engine_display_owasp_profile_selection();
    led_control_run_effect(led_control_game_event_red_team_turn);
  }
}

void game_engine_handle_server_data(char* ble_data) {
  int ble_command = ble_data[0];
  int command_value = ble_data[1];

  switch (ble_command) {
    case BLE_COMMAND_RESPONSE_SELECTED:
      ESP_LOGI(TAG_GAME_ENGINE_MODULE, "BLE_COMMAND_RESPONSE_SELECTED");
      current_game_state.opponent_action = command_value;

      game_engine_handle_battle_round_winner();
      break;
    case BLE_COMMAND_PROFILE_SELECTED:
      ESP_LOGI(TAG_GAME_ENGINE_MODULE, "BLE_COMMAND_PROFILE_SELECTED");
      current_game_state.opponent_profile =
          game_engine_get_owasp_profile(command_value);
      current_game_state.attacker_profile =
          game_engine_get_owasp_profile(command_value);
      oled_driver_display_fadeout();
      oled_driver_clear(OLED_DISPLAY_NORMAL);
      oled_driver_display_text_center(0, "Red Team Profile",
                                      OLED_DISPLAY_NORMAL);
      int started_page = 2;
      oled_driver_display_text_splited(
          current_game_state.opponent_profile->vuln->name, &started_page,
          OLED_DISPLAY_NORMAL);
      break;
    case BLE_COMMAND_ATTACK_SELECTED: {
      ESP_LOGI(TAG_GAME_ENGINE_MODULE, "BLE_COMMAND_ATTACK_SELECTED");
      current_game_state.attacker_action = command_value;
      oled_driver_clear(OLED_DISPLAY_NORMAL);
      oled_driver_display_text(0, "Red Team used", OLED_DISPLAY_NORMAL);
      int started_page = 1;
      if (command_value == 0) {
        oled_driver_display_text_splited(
            current_game_state.opponent_profile->action1->attack->name,
            &started_page, OLED_DISPLAY_NORMAL);
      } else if (command_value == 1) {
        oled_driver_display_text_splited(
            current_game_state.opponent_profile->action2->attack->name,
            &started_page, OLED_DISPLAY_NORMAL);
      } else {
        ESP_LOGW(TAG_GAME_ENGINE_MODULE, "Invalid command value");
      }

      led_control_run_effect(led_control_game_event_blue_team_turn);

      current_game_state.game_state = GAME_STATE_OPPONENT_RESPONSE_SELECTION;
      game_engine_display_owasp_profile_response();
      break;
    }
    default:
      break;
  }
}

void game_engine_cb_paired_devices(game_team_color_t team_device) {
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Paired devices");
  oled_driver_clear(OLED_DISPLAY_NORMAL);

  if (team_device == GAME_TEAM_BLUE) {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Paired devices - Blue team");

    screen_module_display_game_paired_client();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    screen_module_display_game_vs_screen();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    oled_driver_display_fadeout();
    screen_module_display_game_waiting_profile();
  } else {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Paired devices - Red team");
    current_game_state.game_state = GAME_STATE_ATTACKER_PROFILE_SELECTION;

    screen_module_display_game_paired_server();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    screen_module_display_game_vs_screen();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    oled_driver_display_fadeout();
    game_engine_display_owasp_profile_selection();
  }

  led_control_run_effect(led_control_game_event_red_team_turn);
}

void game_engine_display_team_selection() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);

  if (current_option > 1) {
    current_option = 0;
  }
  oled_driver_display_text_center(0, "Select Team", OLED_DISPLAY_NORMAL);

  if (current_option == 0) {
    screen_module_display_game_blue_team_logo();
    oled_driver_display_text_center(7, "Defender", OLED_DISPLAY_NORMAL);
  } else {
    screen_module_display_game_red_team_logo();
    oled_driver_display_text_center(7, "Attackers", OLED_DISPLAY_NORMAL);
  }
}

void game_engine_display_owasp_profile_selection() {
  int total_profiles = GAME_OWASP_PROFILES_COUNT;

  int profiles_per_page = 3;
  int started_page = 1;

  int upper_limit = current_option + profiles_per_page;

  if (current_option > total_profiles) {
    current_option = 0;
    upper_limit = profiles_per_page;
  }

  if (upper_limit > total_profiles) {
    upper_limit = total_profiles;
  }

  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(0, "Select Profile", OLED_DISPLAY_NORMAL);

  for (int i = current_option; i < upper_limit; i++) {
    int profile_index = i % total_profiles;

    game_owasp_profile_t* profile =
        game_engine_get_owasp_profile(profile_index);

    if (i == current_option) {
      char item_text[strlen(profile->vuln->cwe) + strlen(profile->vuln->name) +
                     3];
      sprintf(item_text, "> %s", profile->vuln->name);

      oled_driver_display_text_splited(item_text, &started_page,
                                       OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_text_splited(profile->vuln->name, &started_page,
                                       OLED_DISPLAY_NORMAL);
    }
    started_page++;
  }
}

void game_engine_display_owasp_profile_attacks() {
  led_control_run_effect(led_control_game_event_red_team_turn);
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(0, "Select Attack", OLED_DISPLAY_NORMAL);
  int started_page = 1;
  int random_option = generate_random_float();
  if (random_option == 0) {
    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action1->attack->name,
        &started_page,
        (current_option == 0) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);

    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action2->attack->name,
        &started_page,
        (current_option == 1) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
  } else {
    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action2->attack->name,
        &started_page,
        (current_option == 0) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action1->attack->name,
        &started_page,
        (current_option == 1) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
  }
}

void game_engine_display_owasp_profile_response() {
  led_control_run_effect(led_control_game_event_blue_team_turn);
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(0, "Select Response", OLED_DISPLAY_NORMAL);
  int started_page = 1;
  int random_option = generate_random_float();
  if (random_option == 0) {
    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action1->response->name,
        &started_page,
        (current_option == 0) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action2->response->name,
        &started_page,
        (current_option == 1) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
  } else {
    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action2->response->name,
        &started_page,
        (current_option == 0) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
    oled_driver_display_text_splited(
        current_game_state.attacker_profile->action1->response->name,
        &started_page,
        (current_option == 1) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
  }
}

game_owasp_profile_t* game_engine_get_owasp_profile(int index) {
  if (index >= 0 &&
      index < sizeof(owasp_game_profiles) / sizeof(owasp_game_profiles[0])) {
    return &owasp_game_profiles[index];
  }
  return &owasp_game_profiles[0];
}

static int game_engine_get_owasp_profile_index(game_owasp_profile_t* profile) {
  for (int i = 0;
       i < sizeof(owasp_game_profiles) / sizeof(owasp_game_profiles[0]); i++) {
    if (strcmp(profile->vuln->name, owasp_game_profiles->vuln[i].name) == 0) {
      return i;
    }
  }
  return 0;
}

void game_engine_cb_disconnected_devices(game_team_color_t team_device) {
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Disconnected devices");
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(4, "Disconnected", OLED_DISPLAY_NORMAL);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  esp_restart();
}
