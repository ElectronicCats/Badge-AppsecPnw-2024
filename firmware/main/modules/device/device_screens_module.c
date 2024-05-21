#include "modules/device/device_screens_module.h"
#include "drivers/oled_ssd1306_driver.h"

// 'Isotipo-EC_Negro', 64x32px
static unsigned char epd_bitmap_Isotipo_EC_Negro[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00,
    0x00, 0x07, 0xe0, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x1f, 0xf0, 0x00,
    0x00, 0x0f, 0xfc, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0x00,
    0x00, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x01, 0xff, 0xf0, 0x00,
    0x00, 0x0f, 0xff, 0xc7, 0xe3, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0xff,
    0xff, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00,
    0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0xff,
    0xff, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00,
    0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff,
    0xff, 0xff, 0xe0, 0x00, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0,
    0x1f, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xf8, 0x3f, 0xff, 0xf0, 0xff,
    0xff, 0x0f, 0xff, 0xfc, 0x3f, 0xff, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xfc,
    0x1f, 0xff, 0xf8, 0x3f, 0xfc, 0x1f, 0xff, 0xf8, 0x0f, 0x7f, 0xfc, 0x1f,
    0xf8, 0x3f, 0xfe, 0xf0, 0x3e, 0x1f, 0xfe, 0x1f, 0xf8, 0x7f, 0xf8, 0x7c,
    0x7f, 0xff, 0xff, 0xcf, 0xf3, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0x3e, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x7c, 0x00, 0x1f, 0xff, 0xff,
    0xff, 0xff, 0xf8, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,
    0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xf0, 0x1f, 0xff,
    0xff, 0xf8, 0x0f, 0xfc, 0x3f, 0x80, 0x07, 0xff, 0xff, 0xe0, 0x01, 0xfc,
    0x1f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xf8, 0x06, 0x00, 0x00, 0x03,
    0xc0, 0x00, 0x00, 0x60};

void device_screens_module_version() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "AppsecPnw", OLED_DISPLAY_INVERTED);
  oled_driver_display_text_center(4, "Version: 1.0.0", OLED_DISPLAY_NORMAL);
}

void device_screens_module_licence() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  int current_page = 6;
  oled_driver_display_text_center(0, "LICENCE", OLED_DISPLAY_INVERTED);
  oled_driver_display_text_center(2, "Firmware:", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(3, "GNU GPL 3.0", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(5, "Hardware:", OLED_DISPLAY_NORMAL);
  oled_driver_display_text_splited("CERN Open Hardware Licence v1.2",
                                   &current_page, OLED_DISPLAY_NORMAL);
}

void device_screens_module_credits() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "Developed by", OLED_DISPLAY_INVERTED);
  oled_driver_display_text_center(1, "Electronic Cats", OLED_DISPLAY_INVERTED);
  oled_driver_display_bitmap(32, 16, epd_bitmap_Isotipo_EC_Negro, 64, 32,
                             OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(7, "X: kevlem97", OLED_DISPLAY_NORMAL);
}

void device_screens_module_legal() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  int current_page = 1;
  oled_driver_display_text_center(0, "LEGAL", OLED_DISPLAY_INVERTED);
  oled_driver_display_text_splited("The user assumes all responsibility",
                                   &current_page, OLED_DISPLAY_NORMAL);
  oled_driver_display_text_splited("Agree to use it ethically", &current_page,
                                   OLED_DISPLAY_NORMAL);
}
