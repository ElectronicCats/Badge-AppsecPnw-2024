#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "display.h"
#include "ble_profiles.h"
#include "ble_client_profiles.h"
#include "engine.h"
#include "keyboard.h"
#include "memory.h"


#define TAG_SCREEN "DISPLAY"

SSD1306_t dev;
uint8_t selected_option;
Layer previous_layer;
Layer current_layer;
int num_items;
uint8_t bluetooth_devices_count;

GameInformation current_game;

static void display_center_text(const char* text, int page, int invert);

void display_init(void) {
    ESP_LOGI(TAG_SCREEN, "Initialize I2C bus");
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);

    selected_option = 0;
    previous_layer = LAYER_MAIN_MENU;
    current_layer = LAYER_MAIN_MENU;
    num_items = 0;
    bluetooth_devices_count = 0;

    // wifi_sniffer_register_cb(display_wifi_sniffer);
    // bluetooth_scanner_register_cb(display_bluetooth_scanner);
    ssd1306_bitmaps(&dev, 0, 0, epd_bitmap_owasp_logo, 128, 64, NO_INVERT);
}

void display_clear() {
    ssd1306_clear_screen(&dev, false);
}

void display_show() {
    ssd1306_show_buffer(&dev);
}

/// @brief Display text on the screen
/// @param text
/// @param text_size

/// @param page
/// @param invert
void display_text(const char* text, int x, int page, int invert) {
    ssd1306_display_text(&dev, page, text, x, invert);
}

/// @brief Clear a line on the screen
/// @param page
/// @param invert
void display_clear_line(int page, int invert) {
    ssd1306_clear_line(&dev, page, invert);
}

/// @brief Add empty strings at the beginning and end of the array
/// @param array
/// @param length
/// @return
char** add_empty_strings(char** array, int length) {
    char** newArray = malloc((length + 2) * sizeof(char*));

    // Add the empty string at the beginning
    newArray[0] = strdup("");

    // Copy the original array
    for (int i = 0; i < length; i++) {
        newArray[i + 1] = strdup(array[i]);
    }

    // Add the empty string at the end
    newArray[length + 1] = strdup("");

    num_items = length + 2;

    return newArray;
}

char** remove_srolling_text_flag(char** items, int length) {
    char** newArray = malloc((length - 1) * sizeof(char*));

    for (int i = 0; i < length - 1; i++) {
        newArray[i] = strdup(items[i + 1]);
        // ESP_LOGI(TAG_SCREEN, "Item: %s", newArray[i]);
    }
    // ESP_LOGI(TAG_SCREEN, "Number of items: %d", length - 1);

    num_items = length + 1;

    return newArray;
}

char** get_menu_items() {
    num_items = 0;
    char** submenu = menu_items[current_layer];
    if (submenu != NULL) {
        while (submenu[num_items] != NULL) {
            // ESP_LOGI(TAG_SCREEN, "Item: %s", submenu[num_items]);
            num_items++;
        }
    }
    // ESP_LOGI(TAG_SCREEN, "Number of items: %d", num_items);

    if (num_items == 0) {
        return NULL;
    }

    if (strcmp(submenu[0], SCROLLING_TEXT) == 0) {
        return submenu;
    }

    return add_empty_strings(menu_items[current_layer], num_items);
}

void display_menu_items(char** items) {
    // Show only 3 options at a time in the following order:
    // Page 1: Option 1
    // Page 3: Option 2 -> selected option
    // Page 5: Option 3

    display_clear();
    int page = 1;
    char *terminal_user = "appsec@pwn";
    ssd1306_display_text(&dev, page, terminal_user, strlen(terminal_user), NO_INVERT);
    page++;
    for (int i = 0; i < 3; i++) {
        char* name = (char*)malloc(20);
        ESP_LOGI(TAG_SCREEN, "Item: %s", items[i]);
        strcpy(name, items[i+1]);
        
        // if (i == selected_option) {
        //     sprintf(name, "> %s", items[i + selected_option]);
        //     char *prefix = "> ";
        //     char item_text[strlen(prefix) + strlen(name) + 1];
        //     strcpy(item_text, prefix);
        //     strcat(item_text, name);
        //     ssd1306_display_text(&dev, page, item_text, strlen(item_text), INVERT);
        // } else {
        //     ssd1306_display_text(&dev, page, name, strlen(name), NO_INVERT);
        // }
        if (selected_option == i){
            char *prefix = "> ";
            char item_text[strlen(prefix) + strlen(name) + 1];
            strcpy(item_text, prefix);
            strcat(item_text, name);
            ssd1306_display_text(&dev, page, item_text, strlen(item_text), INVERT);
        } else {
            ssd1306_display_text(&dev, page, name, strlen(name), NO_INVERT);
        }

        //ssd1306_display_text(&dev, page, name, strlen(name), NO_INVERT);
        page += 2;
    }

    //display_selected_item();
}

void display_scrolling_text(char** text) {
    uint8_t startIdx = (selected_option >= 7) ? selected_option - 6 : 0;
    selected_option = (num_items - 2 > 7 && selected_option < 6) ? 6 : selected_option;
    display_clear();
    // ESP_LOGI(TAG_SCREEN, "num: %d", num_items - 2);

    for (uint8_t i = startIdx; i < num_items - 2; i++) {
        // ESP_LOGI(TAG_SCREEN, "Text[%d]: %s", i, text[i]);
        if (i == selected_option) {
            //display_text(text[i], 0, (i - startIdx), NO_INVERT);  // Change it to INVERT to debug
            ssd1306_display_text(&dev, (i - startIdx), text[i], 0,  NO_INVERT);
        } else {
            //display_text(text[i], 0, (i - startIdx), NO_INVERT);
            ssd1306_display_text(&dev, (i - startIdx), text[i], 0,  NO_INVERT);
        }
    }
}

void display_menu() {
    char** items = get_menu_items();

    if (items == NULL) {
        ESP_LOGW(TAG_SCREEN, "Options is NULL");
        return;
    }

    if (strcmp(items[0], SCROLLING_TEXT) == 0) {
        char** text = remove_srolling_text_flag(items, num_items);
        display_scrolling_text(text);
    } else {
        display_menu_items(items);
    }
}

// void display_wifi_sniffer(wifi_sniffer_record_t record) {
//     char* channel_str = (char*)malloc(16);
//     char* ssid_str = (char*)malloc(50);
//     char* rssi_str = (char*)malloc(16);
//     char* addr_str = (char*)malloc(30);
//     char* hash_str = (char*)malloc(16);
//     char* htci_str = (char*)malloc(16);
//     char* sn_str = (char*)malloc(16);
//     char* time_str = (char*)malloc(16);

//     sprintf(channel_str, "Channel=%d", record.channel);
//     display_text("WiFi Sniffer    ", 0, 0, INVERT);
//     display_clear_line(1, NO_INVERT);
//     display_text(channel_str, 0, 1, NO_INVERT);

//     if (record.ssid == NULL && record.timestamp == 0) {
//         return;
//     }

//     sprintf(ssid_str, "SSID=%s", record.ssid);
//     sprintf(addr_str, "ADDR=%02x:%02x:%02x:%02x:%02x:%02x", record.addr[0], record.addr[1], record.addr[2], record.addr[3], record.addr[4], record.addr[5]);
//     sprintf(hash_str, "Hash=%s", record.hash);
//     sprintf(rssi_str, "RSSI=%d", record.rssi);
//     sprintf(htci_str, "HTCI=%s", record.htci);
//     sprintf(sn_str, "SN=%d", record.sn);
//     sprintf(time_str, "Time=%d", (int)record.timestamp);

//     display_clear();
//     display_text("WiFi Sniffer    ", 0, 0, INVERT);
//     display_text(ssid_str, 0, 2, NO_INVERT);
//     display_text(addr_str, 0, 3, NO_INVERT);
//     display_text(hash_str, 0, 4, NO_INVERT);
//     display_text(rssi_str, 0, 5, NO_INVERT);
//     display_text(htci_str, 0, 6, NO_INVERT);
//     // display_text(sn_str, 0, 6, NO_INVERT);
//     display_text(time_str, 0, 7, NO_INVERT);

//     ESP_LOGI(TAG_SCREEN,
//              "ADDR=%02x:%02x:%02x:%02x:%02x:%02x, "
//              "SSID=%s, "
//              "TIMESTAMP=%d, "
//              "HASH=%s, "
//              "RSSI=%02d, "
//              "SN=%d, "
//              "HT CAP. INFO=%s",
//              record.addr[0], record.addr[1], record.addr[2], record.addr[3], record.addr[4],
//              record.addr[5], record.ssid, (int)record.timestamp, record.hash, record.rssi, record.sn, record.htci);
// }

void display_in_development_banner() {
    ssd1306_display_text(&dev, 4, "In development", 15, NO_INVERT);
}

void ble_game_paring() {
    current_game.game_state = PAIRING;

    ssd1306_fadeout(&dev);
    ssd1306_clear_screen(&dev, false);
    ssd1306_bitmaps(&dev, 0, 0, epd_bitmap_ble_logo, 16, 16, NO_INVERT);
    ssd1306_display_text(&dev, 4, "Pairing mode...", 15, NO_INVERT);
    init_ble_task();
}

void ble_game_pairing_client() {
    current_game.game_state = PAIRING;

    ssd1306_fadeout(&dev);
    ssd1306_clear_screen(&dev, false);
    ssd1306_bitmaps(&dev, 0, 0, epd_bitmap_ble_logo, 16, 16, NO_INVERT);
    ssd1306_display_text(&dev, 4, "Pairing server...", 17, NO_INVERT);
    init_ble_client_task();

}

void ble_game_paired() {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 4, "BLUE Device", 11, NO_INVERT);
    ssd1306_display_text(&dev, 5, "Paired", 6, NO_INVERT);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ssd1306_fadeout(&dev);
    ssd1306_bitmaps(&dev, 0, 0, epd_bitmap_vs_logo, 128, 64, NO_INVERT);
    display_ble_owasp_profile();
}

void ble_game_paired_client() {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 4, "RED Device", 11, NO_INVERT);
    ssd1306_display_text(&dev, 5, "Paired", 6, NO_INVERT);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ssd1306_fadeout(&dev);
    ssd1306_bitmaps(&dev, 0, 0, epd_bitmap_vs_logo, 128, 64, NO_INVERT);
    display_ble_client_waiting_profile();
}


void ble_game_client_profile_show() {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 4, "Profile", 7, NO_INVERT);
    ssd1306_display_text(&dev, 5, "Selection", 10, NO_INVERT);

}

void display_handle_device(ButtonType button) {
    switch (button){
        case BUTTON_UP:
            display_device_game_selection();
            break;
        case BUTTON_DOWN:
            display_device_game_selection();
            break;
        case BUTTON_LEFT:
            current_layer = SETTINGS_MENU_SYSTEM;
            display_menu();
            break;
        case BUTTON_RIGHT:
            ESP_LOGI(TAG_SCREEN, "TEAM Selected option: %d", selected_option);
            int32_t *selected_team = (int32_t)selected_option;
            write_int32_nvs(&selected_option);
            if (selected_option == 0){
                ble_game_pairing_client();
            }else{
                ble_game_paring();
            }
            break;
        case BUTTON_BOOT:
        default:
            break;
    }
    
}

void display_handle_game_state(ButtonType button){
    switch (current_game.game_state) {
        case PAIRING:
            break;
        case PROFILE_SELECTOR:
            switch(button){
                case BUTTON_UP:
                    display_ble_owasp_profile();
                    break;
                case BUTTON_DOWN:
                    display_ble_owasp_profile();
                    break;
                case BUTTON_LEFT:
                    break;
                case BUTTON_RIGHT: {
                    current_game.profile = getOWASPProfile(selected_option);
                    // uint8_t *command[sizeof(current_game.profile->vuln->cwe)+1];
                    // create_owasp_commmand(&command, current_game.profile);
                    // Create Command:
                    // 0x01 - Command
                    // 0x02 - OWASP Index
                    uint8_t command[2];
                    command[0] = 1;
                    command[1] = selected_option;
                    //command[0] = '1';
                    //command[1] = selected_option;
                    ESP_LOGI(TAG_SCREEN, "-Selected option: %d", selected_option);
                    send_ble_data(&command, sizeof(command));
                    display_ble_owasp_profile_attacks();
                    break;
                }
                case BUTTON_BOOT:
                default:
                    break;
            }
            break;
        case WAITING_ATTACK:
            switch(button) {
                case BUTTON_UP:
                    display_ble_owasp_profile_attacks();
                    break;
                case BUTTON_DOWN:
                    display_ble_owasp_profile_attacks();
                    break;
                case BUTTON_LEFT:
                    break;
                case BUTTON_RIGHT:
                    display_ble_sending_attack();
                    break;
                case BUTTON_BOOT:
                default:
                    break;
            }
            break;
        case TEAM_SELECTION: {
            switch (button){
                case BUTTON_UP:
                    display_device_game_selection();
                    break;
                case BUTTON_DOWN:
                    display_device_game_selection();
                    break;
                case BUTTON_LEFT:
                    break;
                case BUTTON_RIGHT:
                    ESP_LOGI(TAG_SCREEN, "TEAM Selected option: %d", selected_option);
                    
                    if (selected_option == 0){
                        // BLUE TEAM
                        ble_game_pairing_client();
                    }else{
                        // RED TEAM
                        ble_game_paring();
                    }
                    break;
                case BUTTON_BOOT:
                default:
                    break;
            }
            break;
        }
        case SEND_ATTACK:
        case WAITING_RESPONSE:
        case FINISHED:
        case GAME_OVER:
        default:
            break;
    }
}

void display_ble_owasp_profile() {
    current_game.game_state = PROFILE_SELECTOR;
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0,"Select profile", 15, NO_INVERT);
    //ssd1306_bitmaps(&dev, 112, 48, epd_bitmap_ble_logo, 16, 16, NO_INVERT);
    int limit_page = 6;
    if (selected_option > 10) {
        limit_page = 6;
        selected_option = 0;
    }
    for (int i = (int)selected_option; i < limit_page+(int)selected_option; i++) {
        OWASPProfile* profile = getOWASPProfile(i);
        if(i == selected_option){
            char *prefix = "> ";
            char item_text[strlen(prefix) + strlen(profile->vuln->cwe) + 1];
            strcpy(item_text, prefix);
            strcat(item_text, profile->vuln->cwe);
            ssd1306_display_text(&dev, (i+1) - (int)selected_option, item_text, strlen(item_text), NO_INVERT);
        }else{
            ssd1306_display_text(&dev, (i+1) - (int)selected_option, profile->vuln->cwe, strlen(profile->vuln->cwe), NO_INVERT);
        }
    }
}

void display_ble_client_waiting_profile() {
    current_game.game_state = PROFILE_SELECTOR;
    ssd1306_display_text(&dev, 3,"Waiting", 8, NO_INVERT);
    ssd1306_display_text(&dev, 4,"Profile", 8, NO_INVERT);
    ssd1306_display_text(&dev, 5,"Selection", 10, NO_INVERT);
}

void display_ble_owasp_profile_attacks() {
    current_game.game_state = WAITING_ATTACK;
    ssd1306_clear_screen(&dev, false);
    ESP_LOGI(TAG_SCREEN, "Selected option: %s", current_game.profile->vuln->name);
    ssd1306_display_text(&dev, 1, "Select attack", 13, NO_INVERT);

    if (selected_option > 2) {
        selected_option = 0;
    }

    if (selected_option == 0) {
        ssd1306_display_text(&dev, 2, current_game.profile->action1->attack->identifier, strlen(current_game.profile->action1->attack->identifier), INVERT);
        ssd1306_display_text(&dev, 4, current_game.profile->action2->attack->identifier, strlen(current_game.profile->action2->attack->identifier), NO_INVERT);
    }else{
        ssd1306_display_text(&dev, 2, current_game.profile->action1->attack->identifier, strlen(current_game.profile->action1->attack->identifier), NO_INVERT);
        ssd1306_display_text(&dev, 4, current_game.profile->action2->attack->identifier, strlen(current_game.profile->action2->attack->identifier), INVERT);
    }
}

void display_ble_sending_attack() {
    current_game.game_state = SEND_ATTACK;
    ssd1306_fadeout(&dev);
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 4, "Sending attack", 14, NO_INVERT);
    if (selected_option == 0) {
        uint8_t *command[sizeof(current_game.profile->action1->attack->identifier)+1];
    
        create_owasp_commmand_identifier(&command, current_game.profile->action1->attack->identifier);
        send_ble_data(&command, sizeof(current_game.profile->action1->attack->identifier)+1);
    }else{
        uint8_t *command[sizeof(current_game.profile->action2->attack->identifier)+1];
    
        create_owasp_commmand_identifier(&command, current_game.profile->action2->attack->identifier);
        send_ble_data(&command, sizeof(current_game.profile->action2->attack->identifier)+1);
    }
    ESP_LOGI(TAG_SCREEN, "Attack sent");
    display_waiting_response();
}

void display_waiting_response() {
    current_game.game_state = WAITING_RESPONSE;
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 4, "Waiting response", 16, NO_INVERT);
}

void display_blue_team_selection() {
    ssd1306_display_text(&dev, 1, "Select Team", 12, NO_INVERT);
    ssd1306_bitmaps(&dev, 48, 16, epd_bitmap_blue_team_logo, 32, 32, NO_INVERT);

    ssd1306_display_text(&dev, 6, "    BLUE TEAM   ", 16, INVERT);
    ssd1306_display_text(&dev, 7, "    Defenders   ", 16, NO_INVERT);
}

void display_read_team_selection() {
    ssd1306_display_text(&dev, 1, "Select Team", 12, NO_INVERT);
    ssd1306_bitmaps(&dev, 48, 16, epd_bitmap_red_team_logo, 32, 32, NO_INVERT);

    ssd1306_display_text(&dev, 6, "    RED  TEAM   ", 16, INVERT);
    ssd1306_display_text(&dev, 7, "    Attackers   ", 16, NO_INVERT);
}



void display_device_game_selection() {
    current_game.game_state = TEAM_SELECTION;
    ssd1306_clear_screen(&dev, false);

    ESP_LOGI(TAG_SCREEN, "Selected option TEAM SELECTION: %d", selected_option);

    if (selected_option > 1 || selected_option < 0) {
        selected_option = 0;
    }

    if (selected_option == 0) {
        display_blue_team_selection();
    } else {
        display_read_team_selection();
    }

}

void display_device_type() {
    ssd1306_clear_screen(&dev, false);
    if (selected_option > 1 || selected_option < 0) {
        selected_option = 0;
    }    

    if (selected_option == 0) {
        display_blue_team_selection();
    } else {
        display_read_team_selection();
    }
}
