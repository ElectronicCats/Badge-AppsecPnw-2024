set(drivers "drivers/oled_ssd1306_driver.c"
            "drivers/memory_driver.c"
            "drivers/wifi_driver.c")
set(modules "modules/cross_keyboard_module.c"
            "modules/game_screens_module.c"
            "modules/menu_screens_modules.c"
            "modules/app_screens_module.c"
            "modules/game_engine_module.c")

set(modules_ble "modules/ble_server_module.c"
                "modules/ble_client_module.c"
                "modules/ble/airtags_module.c")

set(modules_wifi  "modules/wifi/wifi_module.c"
                  "modules/wifi/wifi_scanner_module.c"
                  "modules/wifi/wifi_screens_module.c"
                  "modules/wifi/wifi_attack_brod_module.c")

idf_component_register(SRCS "main.c" "${drivers}"  "${modules}" "${modules_ble}" "${modules_wifi}"
                    INCLUDE_DIRS ".")
