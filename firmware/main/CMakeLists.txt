set(drivers "drivers/oled_ssd1306_driver.c"
            "drivers/memory_driver.c")
set(modules "modules/cross_keyboard_module.c"
            "modules/game_screens_module.c"
            "modules/menu_screens_modules.c"
            "modules/app_screens_module.c"
            "modules/game_engine_module.c"
            "modules/ble_server_module.c"
            "modules/ble_client_module.c"
            "modules/ble/airtags_module.c"
            "modules/wifi/wifi_scanner_module.c")

idf_component_register(SRCS "main.c" "${drivers}"  "${modules}"
                    INCLUDE_DIRS ".")
