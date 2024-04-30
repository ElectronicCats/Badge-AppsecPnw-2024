set(drivers "drivers/oled_ssd1306_driver.c")

set(modules "modules/cross_keyboard_module.c"
            "modules/game_screens_module.c"
            "modules/menu_screens_modules.c"
            "modules/game_engine_module.c")

set(modules_game "modules/game/game_ble_client.c"
                  "modules/game/game_ble_server.c")

set(modules_ble "modules/ble/ble_module.c"
                "modules/ble/ble_screens_module.c")

set(modules_wifi  "modules/wifi/wifi_module.c"
                  "modules/wifi/wifi_screens_module.c")

set(modules_zigbee "modules/zigbee/zigbee_module.c"
                  "modules/zigbee/zigbee_screens_module.c")

idf_component_register(SRCS "main.c" "${drivers}" "${modules_game}" "${modules_ble}" "${modules_wifi}" "${modules_zigbee}" "${modules}"
                    INCLUDE_DIRS ".")
