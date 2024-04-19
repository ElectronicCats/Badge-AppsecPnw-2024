#include "drivers/oled_ssd1306_driver.h"
#include "esp_log.h"
#include "modules/bitmaps.h"
#include "modules/game_engine_module.h"
#include "string.h"

void screen_module_display_badge_logo() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_bitmap(0, 0, bitmap_owasp_logo, 128, 64,
                             OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_pairing_server() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Pairing to", 10, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Defender", 8, OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_pairing_client() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Pairing to", 10, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Attacker", 8, OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_paired_server() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "BLUE Device", 11, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Paired", 6, OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_paired_client() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "RED Device", 10, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Paired", 6, OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_vs_screen() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_bitmap(0, 0, bitmap_vs_screen_logo, 128, 64,
                             OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_waiting_profile() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(1, "Waiting", 8, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(2, "RED TEAM", 8, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(3, "Profile", 8, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Selection", 10, OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_sending_attack() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(1, "appsec@ exploit", 16, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(2, " Sending Attack", 16, OLED_DISPLAY_NORMAL);
  oled_driver_display_bitmap(48, 32, bitmap_attacker_logo, 32, 32,
                             OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_screen() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_fadeout();
  oled_driver_display_bitmap(32, 0, bitmap_badge_logo, 64, 64,
                             OLED_DISPLAY_NORMAL);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  oled_driver_display_fadeout();
}

void screen_module_display_game_blue_team_logo() {
  oled_driver_display_bitmap(48, 16, bitmap_blue_team_logo, 32, 32,
                             OLED_DISPLAY_NORMAL);
  oled_driver_display_text(6, "   BLUE  TEAM   ", 16, OLED_DISPLAY_INVERTED);
}

void screen_module_display_game_red_team_logo() {
  oled_driver_display_bitmap(48, 16, bitmap_red_team_logo, 32, 32,
                             OLED_DISPLAY_NORMAL);
  oled_driver_display_text(6, "    RED  TEAM   ", 16, OLED_DISPLAY_INVERTED);
}

void screen_module_display_game_waiting_response() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(3, "Waiting", 8, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Response", 8, OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_points_life(int life_points) {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  char life_text[17];
  sprintf(life_text, "        %d", life_points);
  oled_driver_display_text_large(4, life_text, strlen(life_text),
                                 OLED_DISPLAY_NORMAL);
}

void screen_module_display_attacker_profile(game_owasp_profile_t* profile) {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  char* cwe = (char*) malloc(OWASP_CWE_LENGTH);
  char* cwe_text = (char*) malloc(OWASP_NAME_MAX_LENGTH);
  sprintf(cwe, "%s", profile->vuln->cwe);
  sprintf(cwe_text, "%s", profile->vuln->name);
  oled_driver_display_text(0, "Red Team Profile", 17, OLED_DISPLAY_NORMAL);
  oled_driver_display_text(2, cwe, strlen(cwe), OLED_DISPLAY_NORMAL);
  int started_page = 3;
  oled_driver_display_text_splited(cwe_text, &started_page,
                                   OLED_DISPLAY_NORMAL);
  free(cwe);
  free(cwe_text);
}
