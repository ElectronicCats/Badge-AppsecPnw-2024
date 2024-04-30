#include <string.h>
#include "drivers/oled_ssd1306_driver.h"
#include "modules/zigbee/zigbee_bitmaps.h"

void zigbee_screens_display_scanning_animation() {
  oled_driver_clear(OLED_DISPLAY_NORMAL);
  oled_driver_display_text_center(0, "ZIGBEE SNIFFER", OLED_DISPLAY_NORMAL);
  while (true) {
    for (int i = 0; i < zigbee_bitmap_allArray_LEN; i++) {
      oled_driver_display_bitmap(0, 16, zigbee_bitmap_allArray[i], 128, 32,
                                 OLED_DISPLAY_NORMAL);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void zigbee_screens_display_scanning_text(int count) {
  oled_driver_clear_line(7, OLED_DISPLAY_NORMAL);
  char* packets_count = (char*) malloc(17);
  sprintf(packets_count, "Packets: %d", count);
  oled_driver_display_text_center(7, packets_count, OLED_DISPLAY_INVERTED);
}
