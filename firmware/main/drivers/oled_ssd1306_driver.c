#include "drivers/oled_ssd1306_driver.h"

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "ssd1306.h"

SSD1306_t p_driver_screen;

void oled_driver_init() {
#if CONFIG_I2C_INTERFACE
  ESP_LOGI(TAG_OLED_DRIVER, "INTERFACE: I2C");
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
  i2c_master_init(&p_driver_screen, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO,
                  CONFIG_RESET_GPIO);
#endif  // CONFIG_I2C_INTERFACE
#if CONFIG_SPI_INTERFACE
  ESP_LOGI(TAG_OLED_DRIVER, "INTERFACE: SPI");
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_MOSI_GPIO=%d", CONFIG_MOSI_GPIO);
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_SCLK_GPIO=%d", CONFIG_SCLK_GPIO);
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_CS_GPIO=%d", CONFIG_CS_GPIO);
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_DC_GPIO=%d", CONFIG_DC_GPIO);
  ESP_LOGI(TAG_OLED_DRIVER, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
  spi_master_init(&p_driver_screen, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO,
                  CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
#endif  // CONFIG_SPI_INTERFACE

#if CONFIG_FLIP
  p_driver_screen->flip = true;
#endif  // CONFIG_FLIP

#if CONFIG_SSD1306_128x64
  ESP_LOGI(TAG_OLED_DRIVER, "PANEL: 128x64");
  ssd1306_init(&p_driver_screen, 128, 64);
#endif  // CONFIG_SSD1306_128x64
#if CONFIG_SSD1306_128x32
  ESP_LOGI(TAG_OLED_DRIVER, "PANEL: 128x32");
  ssd1306_init(&p_driver_screen, 128, 32);
#endif  // CONFIG_SSD1306_128x32
}

void oled_driver_clear(bool invert) {
  ssd1306_clear_screen(&p_driver_screen, invert);
}

void oled_driver_display_text(int page, char* text, int size, bool invert) {
  ssd1306_display_text(&p_driver_screen, page, text, size, invert);
}

void oled_driver_display_text_large(int page,
                                    char* text,
                                    int size,
                                    bool invert) {
  ssd1306_display_text_x3(&p_driver_screen, page, text, size, invert);
}

void oled_driver_display_bitmap(int x_pos,
                                int y_pos,
                                uint8_t* bitmap,
                                int width,
                                int height,
                                bool invert) {
  ssd1306_bitmaps(&p_driver_screen, x_pos, y_pos, bitmap, width, height,
                  invert);
}

void oled_driver_display_fadeout(void) {
  ssd1306_fadeout(&p_driver_screen);
}

void oled_driver_display_text_scroll_left(int page,
                                          char* text,
                                          int size,
                                          bool invert) {
  ssd1306_display_text(&p_driver_screen, page, text, size, invert);
  ssd1306_hardware_scroll(&p_driver_screen, SCROLL_LEFT);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  ssd1306_hardware_scroll(&p_driver_screen, SCROLL_STOP);
}

void oled_driver_display_text_scroll_right(int page,
                                           char* text,
                                           int size,
                                           bool invert) {
  ssd1306_display_text(&p_driver_screen, page, text, size, invert);
  ssd1306_hardware_scroll(&p_driver_screen, SCROLL_RIGHT);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  ssd1306_hardware_scroll(&p_driver_screen, SCROLL_STOP);
}

void oled_driver_software_scroll(int page_start, int page_end) {
  if (page_end > 0) {
    ssd1306_software_scroll(&p_driver_screen, page_start, page_end);
  } else {
    ssd1306_software_scroll(&p_driver_screen, page_start,
                            (p_driver_screen._pages - 1));
  }
}

void oled_driver_display_scroll_text(char* text, bool invert) {
  ssd1306_scroll_text(&p_driver_screen, text, strlen(text), invert);
}

void oled_driver_clear_line(int page, bool invert) {
  ssd1306_clear_line(&p_driver_screen, page, invert);
}

void oled_driver_display_text_center(int page, char* text, bool invert) {
  int text_length = strlen(text);
  int max_char_len = MAX_LINE_CHAR;
  if (text_length > max_char_len) {
    ESP_LOGE(TAG_OLED_DRIVER, "Text too long to center");
    oled_driver_display_text(page, text, text_length, invert);
    return;
  }

  // We need to know if the text is odd or even
  int text_center = (max_char_len - text_length) / 2;
  char text_centered[MAX_LINE_CHAR] = "";
  for (int i = 0; i < text_center; i++) {
    strcat(text_centered, " ");
  }
  strcat(text_centered, text);
  oled_driver_display_text(page, text_centered, strlen(text_centered), invert);
}

void oled_driver_display_text_splited(const char* p_text,
                                      int* p_started_page,
                                      int invert) {
  if (strlen(p_text) > MAX_LINE_CHAR) {
    char temp[50];
    strncpy(temp, p_text, 50);

    char* token = strtok(temp, " ");
    char current_line[MAX_LINE_CHAR] = "";
    while (token != NULL) {
      if (strlen(current_line) + strlen(token) + 1 <= MAX_LINE_CHAR) {
        if (strlen(current_line) > 0) {
          strcat(current_line, " ");
        }
        strcat(current_line, token);
      } else {
        oled_driver_display_text(*p_started_page, current_line,
                                 strlen(current_line), invert);
        (*p_started_page)++;
        strcpy(current_line, token);
      }
      token = strtok(NULL, " ");
    }
    if (strlen(current_line) > 0) {
      oled_driver_display_text(*p_started_page, current_line,
                               strlen(current_line), invert);
      (*p_started_page)++;
    }
  } else {
    oled_driver_display_text(*p_started_page, p_text, strlen(p_text), invert);
    (*p_started_page)++;
  }
}
