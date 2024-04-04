set(component_srcs "engine.c" "keyboard.c" "ble_profiles.c" "display.c" )

idf_component_register(SRCS "main.c" "${component_srcs}"
                    INCLUDE_DIRS "." "include")
