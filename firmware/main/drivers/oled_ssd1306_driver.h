#include "ssd1306.h"
#ifndef OLED_DRIVER_H
  #define OLED_DRIVER_H
  #define TAG_OLED_DRIVER  "OLED_DRIVER"
  #define MAX_PAGE         8
  #define MAX_LINE_CHAR    17
  #define MAX_WORDS_SCREEN 6
/**
 * @brief Emum for the OLED invert state.
 * Invert the background and foreground color of the OLED display
 */
typedef enum {
  OLED_DISPLAY_NORMAL = 0,
  OLED_DISPLAY_INVERTED,
} OLED_DISPLAY_INVERT;

/**
 * @brief Initialize the OLED display
 *
 */
void oled_driver_init();

/**
 * @brief Clear the OLED display
 *
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_clear(bool invert);

/**
 * @brief Display text on the OLED display
 *
 * @param page Page to display the text on the OLED display
 * @param text Text to display on the OLED display
 * @param size Size of the text
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_display_text(int page, char* text, int size, bool invert);

/**
 * @brief Display large text on the OLED display
 *
 * @param page Page to display the text on the OLED display
 * @param text Text to display on the OLED display
 * @param size Size of the text
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_display_text_large(int page,
                                    char* text,
                                    int size,
                                    bool invert);

/**
 * @brief Display bitmap on the OLED display
 *
 * @param x_pos X position of the bitmap
 * @param y_pos Y position of the bitmap
 * @param bitmap Bitmap to display on the OLED display
 * @param width Width of the bitmap
 * @param height Height of the bitmap
 * @param invert Invert the background and foreground color of the OLED display
 */

void oled_driver_display_bitmap(int x_pos,
                                int y_pos,
                                uint8_t* bitmap,
                                int width,
                                int height,
                                bool invert);

/**
 * @brief Fade out the OLED display by reducing the brightness of the display
 *
 */
void oled_driver_display_fadeout(void);

/**
 * @brief Display scrolling left text on the OLED display
 *
 * @param page Page to display the text on the OLED display
 * @param text Text to display on the OLED display
 * @param size Size of the text
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_display_text_scroll_left(int page,
                                          char* text,
                                          int size,
                                          bool invert);

/**
 * @brief Display scrolling right text on the OLED display
 *
 * @param page Page to display the text on the OLED display
 * @param text Text to display on the OLED display
 * @param size Size of the text
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_display_text_scroll_right(int page,
                                           char* text,
                                           int size,
                                           bool invert);

/**
 * @brief Clear the line on the OLED display
 *
 * @param page The page to clear
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_clear_line(int page, bool invert);

/**
 * @brief Software scroll the OLED display. If the end page is greater than 0,
 * the OLED display will scroll from the start page to the end page. Otherwise,
 * the OLED display will scroll from the start page to the last page.
 *
 * @param page_start Start page to scroll the OLED display
 * @param page_end End page to scroll the OLED display
 */
void oled_driver_software_scroll(int page_start, int page_end);

/**
 * @brief Software scroll display the text on the OLED display
 *
 * @param text Text to display on the OLED display
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_display_scroll_text(char* text, bool invert);

/**
 * @brief Display and split the text on the OLED display
 *
 * @param p_text Pointer of the text to display on the OLED display
 * @param p_started_page Pointer to the index of the page to display the text on
 * the OLED display
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_display_text_splited(const char* p_text,
                                      int* p_started_page,
                                      int invert);

/**
 * @brief Display the text on the center of the OLED display
 *
 * @param page Page to display the text on the OLED display
 * @param text Text to display on the OLED display
 * @param invert Invert the background and foreground color of the OLED display
 */
void oled_driver_display_text_center(int page, char* text, bool invert);
#endif  // OLED_DRIVER_H
