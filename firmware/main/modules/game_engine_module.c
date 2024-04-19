#include "modules/game_engine_module.h"
#include "modules/ble_client_module.h"
#include "modules/ble_server_module.h"
#include "modules/cross_keyboard_module.h"
#include "modules/game_owasp_profile.h"
#include "modules/game_screens_module.h"
#include "modules/menu_screens_modules.h"

#include <esp_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"

game_match_information_t current_game_state;
uint8_t ble_command[GAME_BLE_COMMAND_LENGTH];
int current_option = 0;

void game_engine_state_machine_init() {
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Initializing game engine state machine");
  module_keyboard_update_state(true, game_engine_state_machine);

  current_game_state.game_state = GAME_STATE_TEAM_SELECTION;
  screen_module_display_game_screen();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  game_engine_display_team_selection();
};

void game_engine_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;  // >> 4 to get the button number
  uint8_t button_event =
      button_pressed & 0x0F;  // & 0x0F to get the event number without the mask

  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Game engine state machine from team: %d %d",
           button_name, button_event);
  switch (current_game_state.game_state) {
    case GAME_STATE_TEAM_SELECTION:
      ESP_LOGI(TAG_GAME_ENGINE_MODULE, "GAME_STATE_TEAM_SELECTION");
      switch (button_name) {
        case BUTTON_LEFT:
          if (button_event == BUTTON_DOUBLE_CLICK) {
            ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button left pressed");
            module_keyboard_update_state(false, NULL);
            screen_module_exit_submenu();
            break;
          }

          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          current_game_state.team =
              (current_option == 0) ? GAME_TEAM_BLUE : GAME_TEAM_RED;
          current_game_state.game_state = GAME_STATE_PAIRING_DEVICES;

          current_game_state.attacker.life_points = GAME_DEFAULT_LIFE_POINTS;
          current_game_state.opponent.life_points = GAME_DEFAULT_LIFE_POINTS;

          if (current_game_state.team == GAME_TEAM_BLUE) {
            ble_client_task_begin();
            screen_module_display_game_pairing_client();
          } else {
            ble_server_task_begin();
            screen_module_display_game_pairing_server();
          }
          break;
        case BUTTON_UP:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          game_engine_display_team_selection();
          break;
        case BUTTON_DOWN:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button down pressed");
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
      ESP_LOGI(TAG_GAME_ENGINE_MODULE, "GAME_STATE_ATTACKER_PROFILE_SELECTION");
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
          current_option = (current_option == GAME_OWASP_PROFILES_COUNT)
                               ? current_option
                               : current_option + 1;
          game_engine_display_owasp_profile_selection();
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    case GAME_STATE_ATTACKER_ATTACK_SELECTION:
      ESP_LOGI(TAG_GAME_ENGINE_MODULE, "GAME_STATE_ATTACKER_ATTACK_SELECTION");
      switch (button_name) {
        case BUTTON_LEFT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button left pressed");
          current_game_state.game_state = GAME_STATE_ATTACKER_PROFILE_SELECTION;
          game_engine_display_owasp_profile_selection();
          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_GAME_ENGINE_MODULE,
                   "Button right pressed - Option selected: %d",
                   current_option);
          current_game_state.game_state =
              GAME_STATE_OPPONENT_RESPONSE_SELECTION;
          current_game_state.attacker_action = current_option;
          ble_command[0] = BLE_COMMAND_ATTACK_SELECTED;
          ble_command[1] = current_option;
          ble_server_send_data(ble_command, GAME_BLE_COMMAND_LENGTH);

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
      ESP_LOGI(TAG_GAME_ENGINE_MODULE,
               "GAME_STATE_OPPONENT_RESPONSE_SELECTION");
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
          ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Button up pressed");
          current_option = (current_option == 0) ? 0 : current_option - 1;
          game_engine_display_owasp_profile_response();
          break;
        case BUTTON_DOWN:
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
  } else {
    screen_module_display_game_blue_team_logo();
  }
  oled_driver_display_text(7, "     WINNER", 16, OLED_DISPLAY_NORMAL);
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
    oled_driver_display_text(7, "    DEFENDED    ", 16, OLED_DISPLAY_NORMAL);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    oled_driver_display_text(0, "Attacker", 8, OLED_DISPLAY_NORMAL);
    screen_module_display_game_points_life(
        current_game_state.opponent.life_points);
    if (current_game_state.opponent.life_points <= 0) {
      game_engine_handle_game_over();
      return;
    }

  } else {
    current_game_state.attacker.life_points -= GAME_DEFAULT_ATTACK_POINTS;
    screen_module_display_game_red_team_logo();
    oled_driver_display_text(7, "      PWNED   ", 16, OLED_DISPLAY_NORMAL);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    oled_driver_display_text(0, "Defender", 8, OLED_DISPLAY_NORMAL);
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
  } else {
    current_game_state.game_state = GAME_STATE_ATTACKER_PROFILE_SELECTION;
    game_engine_display_owasp_profile_selection();
  }
}

void game_engine_handle_server_data(char* ble_data) {
  int ble_command = ble_data[0];
  int command_value = ble_data[1];
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "BLE DATA FROM %d", current_game_state.team);
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "notify value %s", ble_data);
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "command value %d", ble_command);
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "value %d", command_value);

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
      oled_driver_display_text(0, "Red Team Profile", 17, OLED_DISPLAY_NORMAL);
      oled_driver_display_text(
          2, current_game_state.opponent_profile->vuln->cwe,
          strlen(current_game_state.opponent_profile->vuln->cwe),
          OLED_DISPLAY_NORMAL);
      int started_page = 4;
      oled_driver_display_text_splited(
          current_game_state.opponent_profile->vuln->name, &started_page,
          OLED_DISPLAY_NORMAL);
      break;
    case BLE_COMMAND_ATTACK_SELECTED: {
      ESP_LOGI(TAG_GAME_ENGINE_MODULE, "BLE_COMMAND_ATTACK_SELECTED");
      current_game_state.attacker_action = command_value;
      oled_driver_clear(OLED_DISPLAY_NORMAL);
      oled_driver_display_text(0, "Red Team used", 17, OLED_DISPLAY_NORMAL);
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
    oled_driver_display_fadeout();
    screen_module_display_game_waiting_profile();
  } else {
    ESP_LOGI(TAG_GAME_ENGINE_MODULE, "Paired devices - Red team");
    current_game_state.game_state = GAME_STATE_ATTACKER_PROFILE_SELECTION;

    screen_module_display_game_paired_server();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    screen_module_display_game_vs_screen();
    oled_driver_display_fadeout();
    game_engine_display_owasp_profile_selection();
  }
}

void game_engine_display_team_selection() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);

  if (current_option > 1) {
    current_option = 0;
  }
  oled_driver_display_text(0, "Select Team", 12, OLED_DISPLAY_NORMAL);

  if (current_option == 0) {
    screen_module_display_game_blue_team_logo();
    oled_driver_display_text(7, "    Defender   ", 16, OLED_DISPLAY_NORMAL);
  } else {
    screen_module_display_game_red_team_logo();
    oled_driver_display_text(7, "    Attackers   ", 16, OLED_DISPLAY_NORMAL);
  }
}

void game_engine_display_owasp_profile_selection() {
  int limit_page = 6;
  if (current_option > GAME_OWASP_PROFILES_COUNT) {
    limit_page = 6;
    current_option = 0;
  }

  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(0, "Select Profile", 16, OLED_DISPLAY_NORMAL);

  for (int i = (int) current_option; i < limit_page + (int) current_option;
       i++) {
    game_owasp_profile_t* profile = game_engine_get_owasp_profile(i);
    if (i == current_option) {
      char* prefix = "> ";
      char item_text[strlen(prefix) + strlen(profile->vuln->cwe) + 1];
      strcpy(item_text, prefix);
      strcat(item_text, profile->vuln->cwe);
      oled_driver_display_text((i + 1) - (int) current_option, item_text,
                               strlen(item_text), OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_text((i + 1) - (int) current_option,
                               profile->vuln->cwe, strlen(profile->vuln->cwe),
                               OLED_DISPLAY_NORMAL);
    }
  }
}

void game_engine_display_owasp_profile_attacks() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(0, "Select Attack", 13, OLED_DISPLAY_NORMAL);
  ESP_LOGI(TAG_GAME_ENGINE_MODULE, "attack1 %s",
           current_game_state.attacker_profile->action1->attack->name);
  int started_page = 1;

  oled_driver_display_text_splited(
      current_game_state.attacker_profile->action1->attack->name, &started_page,
      (current_option == 0) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
  started_page++;
  oled_driver_display_text_splited(
      current_game_state.attacker_profile->action2->attack->name, &started_page,
      (current_option == 1) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
}

void game_engine_display_owasp_profile_response() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(0, "Select Response", 17, OLED_DISPLAY_NORMAL);
  int started_page = 1;
  oled_driver_display_text_splited(
      current_game_state.attacker_profile->action1->response->name,
      &started_page,
      (current_option == 0) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
  oled_driver_display_text_splited(
      current_game_state.attacker_profile->action2->response->name,
      &started_page,
      (current_option == 1) ? OLED_DISPLAY_INVERTED : OLED_DISPLAY_NORMAL);
}

game_owasp_profile_t* game_engine_get_owasp_profile(int index) {
  if (index >= 0 &&
      index < sizeof(owasp_game_profiles) / sizeof(owasp_game_profiles[0])) {
    return &owasp_game_profiles[index];
  }
  return &owasp_game_profiles[0];
}

void string_split(const char* str, char* str_array[], const char* delimiter) {
  char* token;
  int i = 0;
  token = strtok(str, delimiter);
  while (token != NULL) {
    str_array[i] = token;
    token = strtok(NULL, delimiter);
    i++;
  }
}