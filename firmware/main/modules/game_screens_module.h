#include "modules/game_engine_module.h"
#ifndef GAME_SCREENS_MODULE_H
  #define GAME_SCREENS_MODULE_H
  #define TAG_SCREEN_MODULE "GAME_SCREEN_MODULE"

void screen_module_display_badge_logo(void);
void screen_module_display_game_pairing_server(void);
void screen_module_display_game_pairing_client(void);
void screen_module_display_game_paired_server(void);
void screen_module_display_game_paired_client(void);
void screen_module_display_game_vs_screen(void);
void screen_module_display_game_waiting_profile(void);
void screen_module_display_game_sending_attack(void);
void screen_module_display_game_screen(void);
void screen_module_display_game_red_team_logo(void);
void screen_module_display_game_blue_team_logo(void);
void screen_module_display_game_waiting_response(void);
void screen_module_display_game_points_life(int life_points);
void screen_module_display_attacker_profile(game_owasp_profile_t* profile);

void screen_module_display_badge_sith();
void screen_module_display_badge_jedi();

#endif  // GAME_SCREENS_MODULE_H
