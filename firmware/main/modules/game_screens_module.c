#include "drivers/oled_ssd1306_driver.h"
#include "esp_log.h"
#include "modules/bitmaps.h"
#include "modules/game_engine_module.h"
#include "preferences.h"
#include "string.h"

void screen_module_display_badge_logo() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  int what_badge = preferences_get_int("SHOWBADGE", 99);
  int jedi_badge = preferences_get_int("UBADGEJEDI", 99);
  int sith_badge = preferences_get_int("UBADGESITH", 99);
  ESP_LOGI("BADGE", "what_badge: %d", what_badge);
  ESP_LOGI("BADGE", "jedi_badge: %d", jedi_badge);
  ESP_LOGI("BADGE", "sith_badge: %d", sith_badge);

  if (what_badge == 1) {
    oled_driver_display_bitmap(0, 0, michi_bitmap_jedi_1_bg, 128, 64,
                               OLED_DISPLAY_INVERTED);
  } else if (what_badge == 2) {
    oled_driver_display_bitmap(0, 0, epd_bitmap_sith_1, 128, 64,
                               OLED_DISPLAY_INVERTED);
  } else {
    if (jedi_badge == 1) {
      oled_driver_display_bitmap(0, 0, michi_bitmap_jedi_1_bg, 128, 64,
                                 OLED_DISPLAY_INVERTED);
    } else if (sith_badge == 1) {
      oled_driver_display_bitmap(0, 0, epd_bitmap_sith_1, 128, 64,
                                 OLED_DISPLAY_INVERTED);
    } else {
      oled_driver_display_bitmap(0, 0, bitmap_owasp_logo, 128, 64,
                                 OLED_DISPLAY_NORMAL);
    }
  }
}

void screen_module_display_game_pairing_server() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Pairing to", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Defender", OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_pairing_client() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Pairing to", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Attacker", OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_paired_server() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "BLUE Device", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Paired", OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_paired_client() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "RED Device", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(5, "Paired", OLED_DISPLAY_NORMAL);
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
  oled_driver_display_text(1, "Waiting", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(2, "RED TEAM", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(3, "Profile", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Selection", OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_sending_attack() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(0, "appsec@ exploit", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(2, " Sending Attack", OLED_DISPLAY_NORMAL);
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
  oled_driver_display_text_center(6, "BLUE  TEAM", OLED_DISPLAY_INVERTED);
}

void screen_module_display_game_red_team_logo() {
  oled_driver_display_bitmap(48, 16, bitmap_red_team_logo, 32, 32,
                             OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(6, "RED  TEAM", OLED_DISPLAY_INVERTED);
}

void screen_module_display_game_waiting_response() {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text(3, "Waiting", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(4, "Response", OLED_DISPLAY_NORMAL);
}

void screen_module_display_game_points_life(int life_points) {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  char life_text[16];
  sprintf(life_text, "%d", life_points);
  oled_driver_display_text_center(4, life_text, OLED_DISPLAY_NORMAL);
}

void screen_module_display_attacker_profile(game_owasp_profile_t* profile) {
  oled_driver_display_fadeout();
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  char* cwe = (char*) malloc(OWASP_CWE_LENGTH);
  char* cwe_text = (char*) malloc(OWASP_NAME_MAX_LENGTH);
  sprintf(cwe, "%s", profile->vuln->cwe);
  sprintf(cwe_text, "%s", profile->vuln->name);
  oled_driver_display_text(0, "Red Team Profile", OLED_DISPLAY_NORMAL);
  oled_driver_display_text(2, cwe, OLED_DISPLAY_NORMAL);
  int started_page = 3;
  oled_driver_display_text_splited(cwe_text, &started_page,
                                   OLED_DISPLAY_NORMAL);
  free(cwe);
  free(cwe_text);
}

void screen_module_display_badge_sith() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_bitmap(0, 8, epd_bitmap_sith_1, 128, 64,
                             OLED_DISPLAY_INVERTED);
}

void screen_module_display_badge_jedi() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_bitmap(0, 8, michi_bitmap_jedi_1_bg, 128, 64,
                             OLED_DISPLAY_INVERTED);
}
