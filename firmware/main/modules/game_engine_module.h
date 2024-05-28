#include "modules/cross_keyboard_module.h"
#ifndef GAME_ENGINE_MODULE_H
  #define GAME_ENGINE_MODULE_H
  #define TAG_GAME_ENGINE_MODULE     "GAME_ENGINE_MODULE"
  #define GAME_RED_TEAM              0
  #define GAME_BLUE_TEAM             1
  #define GAME_DEFAULT_LIFE_POINTS   30
  #define GAME_DEFAULT_ATTACK_POINTS 10
  #define GAME_BLE_COMMAND_LENGTH    2

  #define OWASP_IDENTIFIER_LENGTH 8
  #define OWASP_CWE_LENGTH        3
  #define OWASP_NAME_MAX_LENGTH   50

  #define PREFERENCE_CONFIGURED_TAG     "GAME_CONFIGURED"
  #define PREFERENCE_PROFILE_LEVEL_TAG  "PROFILE_LEVEL"
  #define PREFERENCE_BPROFILE_LEVEL_TAG "BPROFILE_LEVEL"

typedef enum {
  GAME_SITH_RANGE_APPRENTICE = 0,  // Level 3
  GAME_SITH_RANGE_MASTER,          // Level 6
  GAME_SITH_RANGE_LORD,            // Level 9
  GAME_SITH_RANGE_EMPEROR          // Level 10
} game_sith_range_t;

typedef enum {
  GAME_JEDI_RANGE_YOUNGLING = 0,
  GAME_JEDI_RANGE_PADAWAN,
  GAME_JEDI_RANGE_KNIGHT,
  GAME_JEDI_RANGE_MASTER
} game_jedi_range_t;

/**
 * @brief Enum to represent the game engine state.
 */
typedef enum {
  GAME_STATE_TEAM_SELECTION = 0,
  GAME_STATE_PAIRING_DEVICES,
  GAME_STATE_ATTACKER_PROFILE_SELECTION,
  GAME_STATE_ATTACKER_ATTACK_SELECTION,
  GAME_STATE_OPPONENT_RESPONSE_SELECTION,
} game_engine_state_t;

/**
 * @brief Enum to represent the game BLE command.
 */
typedef enum {
  BLE_COMMAND_PROFILE_SELECTED = 5,
  BLE_COMMAND_ATTACK_SELECTED,
  BLE_COMMAND_RESPONSE_SELECTED
} game_ble_command_t;

/**
 * @brief Enum to represent the game team color.
 */
typedef enum { GAME_TEAM_RED, GAME_TEAM_BLUE } game_team_color_t;

/**
 * @brief Struct to hold the game player information.
 */
typedef struct {
  char username[8];
  int life_points;
  game_team_color_t team;
} game_player_information_t;

/**
 * @brief Struct to hold the OWASP vulns.
 */
typedef struct {
  char cwe[3];
  char name[OWASP_NAME_MAX_LENGTH];
} game_owasp_vulns_t;

/**
 * @brief Struct to hold the OWASP base actions.
 */
typedef struct {
  char identifier[OWASP_IDENTIFIER_LENGTH];
  char name[OWASP_NAME_MAX_LENGTH];
} game_owasp_base_actions_t;

/**
 * @brief Struct to hold the OWASP actions.
 */
typedef struct {
  game_owasp_base_actions_t* attack;
  game_owasp_base_actions_t* response;
} game_owasp_actions_t;

/**
 * @brief Struct to hold the OWASP profile.
 */
typedef struct {
  game_owasp_vulns_t* vuln;
  game_owasp_actions_t* action1;
  game_owasp_actions_t* action2;
} game_owasp_profile_t;

/**
 * @brief Struct to hold the game match information.
 */
typedef struct {
  game_engine_state_t game_state;
  game_player_information_t attacker;
  game_player_information_t opponent;
  game_owasp_profile_t* attacker_profile;
  game_owasp_profile_t* opponent_profile;
  int attacker_action;
  int opponent_action;
  game_team_color_t team;
} game_match_information_t;

/**
 * @brief Get the OWASP profile by index.
 *
 * @param index The index of the OWASP profile.
 * @return The OWASP profile pointer.
 */
game_owasp_profile_t* game_engine_get_owasp_profile(int index);

/**
 * @brief Initialize the game engine state machine.
 *
 * @return void
 */
void game_engine_state_machine_init(void);

/**
 * @brief Handle the game engine state machine.
 *
 * @param button_pressed The button event.
 * @return void
 */
void game_engine_state_machine(button_event_t button_pressed);

/**
 * @brief Show the game team selection screen.
 *
 * @return void
 */
void game_engine_display_team_selection(void);

/**
 * @brief Show the game OWASP profile selection screen.
 *
 * @return void
 */
void game_engine_display_owasp_profile_selection(void);

/**
 * @brief Show the game OWASP attacks selection screen.
 *
 * @return void
 */
void game_engine_display_owasp_profile_attacks(void);

/**
 * @brief Show the game OWASP response selection screen.
 *
 * @return void
 */
void game_engine_display_owasp_profile_response(void);

/**
 * @brief Handle the BLE data received from the server.
 *
 * @return void
 */
void game_engine_handle_server_data(char* ble_data);

/**
 * @brief Callback for the paired devices.
 *
 * @return void
 */
void game_engine_cb_paired_devices(game_team_color_t team_device);

/**
 * @brief Handler for the game winner.
 *
 * @return void
 */
void game_engine_handle_game_over(void);

/**
 * @brief Handler for the game round winner.
 *
 * @return void
 */
void game_engine_handle_battle_round_winner(void);

void game_engine_cb_disconnected_devices(game_team_color_t team_device);
#endif  // GAME_ENGINE_MODULE_H
