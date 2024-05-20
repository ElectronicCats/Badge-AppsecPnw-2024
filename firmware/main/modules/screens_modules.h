#include <stddef.h>
#ifndef SCREENS_MODULE_H
  #define SCREENS_MODULE_H

enum menu_layer_t {
  LAYER_MAIN_MENU = 0,
  LAYER_APPLICATIONS,
  LAYER_SETTINGS,
  LAYER_ABOUT,
  /* Applications */
  LAYER_WIFI_APPS,
  LAYER_BLUETOOTH_APPS,
  LAYER_ZIGBEE_APPS,
  // LAYER_THREAD_APPS,
  //  LAYER_MATTER_APPS,
  //  LAYER_GPS,
  /* WiFi applications */
  LAYER_WIFI_ANALIZER,
  LAYER_WIFI_DEAUTH,
  /* Bluetooth applications */
  LAYER_BLUETOOTH_AIRTAGS_SCAN,
  LAYER_BLUETOOTH_SPAM,
  LAYER_BLUETOOTH_GAME,
  /* Zigbee applications */
  LAYER_ZIGBEE_SNIFFER,
  /* GPS applications */
  // LAYER_GPS_DATE_TIME,
  // LAYER_GPS_LOCATION,
  /* About items */
  LAYER_ABOUT_VERSION,
  LAYER_ABOUT_LICENSE,
  LAYER_ABOUT_CREDITS,
  LAYER_ABOUT_LEGAL,
  /* Settings items */
  LAYER_SETTINGS_DISPLAY,
  LAYER_SETTINGS_DEVICE,
  LAYER_SETTINGS_SOUND,
  LAYER_SETTINGS_SYSTEM,
  /* About submenus */
};

typedef enum menu_layer_t screen_module_layer_t;

enum menu_layer_main_items_t {
  MAIN_MENU_APPLICATIONS = 0,
  MAIN_MENU_SETTINGS,
  MAIN_MENU_ABOUT,
};

enum menu_layer_applications_items_t {
  APPLICATIONS_MENU_WIFI = 0,
  APPLICATIONS_MENU_BLUETOOTH,
  APPLICATIONS_MENU_ZIGBEE,
  APPLICATIONS_MENU_THREAD,
  APPLICATIONS_MENU_MATTER,
  APPLICATIONS_MENU_GPS,
};

enum menu_layer_settings_items_t {
  SETTINGS_MENU_DISPLAY = 0,
  SETTINGS_MENU_DEVICE,
  SETTINGS_MENU_SOUND,
  SETTINGS_MENU_SYSTEM,

};

enum menu_layer_about_items_t {
  ABOUT_MENU_VERSION = 0,
  ABOUT_MENU_LICENSE,
  ABOUT_MENU_CREDITS,
  ABOUT_MENU_LEGAL,
};

enum menu_layer_wifi_items_t {
  // WIFI_MENU_ANALIZER = 0,
  WIFI_MENU_DEAUTH = 0,
};

enum menu_layer_bluetooth_items_t {
  BLUETOOTH_MENU_AIRTAGS_SCAN = 0,
  BLUETOOTH_MENU_SPAM,
  BLUETOOTH_MENU_GAME,
};

enum menu_layer_zigbee_items_t {
  ZIGBEE_MENU_SNIFFER = 0,
};

static char* main_items[] = {
    "Applications",
    "Settings",
    "About",
    NULL,
};

static char* applications_items[] = {
    "WiFi",
    "Bluetooth",
    "Zigbee",
    NULL,
};

static char* settings_items[] = {
    "Display",
    NULL,
};

static char* settings_system_items[] = {"Team", NULL};

static char* about_items[] = {
    "Version", "License", "Credits", "Legal", NULL,
};

static char* version_text[] = {
    "", "", "", " APPSEC v1.0.0", "     BETA", NULL,
};

static char* license_text[] = {
    "", "", "", "  GNU GPL 3.0", NULL,
};

static char* credits_text[] = {
    "Developed by",    "Electronic Cats", "",   "This product is",
    "in a BETA stage", "use at your own", NULL,
};

static char* legal_text[] = {
    "The user",        "assumes all",     "responsibility",   "for the use of",
    "APPSEC and",      "agrees to use",   "it legally and",   "ethically,",
    "avoiding any",    "activities that", "may cause harm,",  "interference,",
    "or unauthorized", "access to",       "systems or data.", NULL,
};

static char* wifi_items[] = {
    //"Analizer",
    "Deauth",
    NULL,
};

static char* bluetooth_items[] = {
    "Trackers scan",
    "Spam",
    "Game",
    NULL,
};

static char* zigbee_items[] = {
    "Zigbee Sniffer",
    NULL,
};

static char* empty_items[] = {
    NULL,
};

// List of menus, it must be in the same order as the enum MenuLayer
static char** menu_items[] = {
    main_items, applications_items, settings_items, about_items,
    /* Applications */
    wifi_items, bluetooth_items,
    zigbee_items,  // Zigbee
    empty_items,
    empty_items,  // Matter
    empty_items,
    /* WiFi applications */
    empty_items,  // WiFi Analizer
    empty_items,  // WiFi Deauth
    /* Bluetooth applications */
    empty_items,  // Bluetooth Airtags scan
    empty_items,  // Bluetooth Game
    /* Zigbee applications */
    empty_items,  // Zigbee Sniffer
    /* Thread applications */
    empty_items,  // Thread CLI
    /* GPS applications */
    empty_items,  // Date & Time
    empty_items,  // Location
    /* About */
    version_text, license_text, credits_text, legal_text,
    /* Settings items */
    empty_items,  // Display
    empty_items,  // device
    empty_items,  // Sound
    empty_items,  // System
};
#endif  // SCREENS_MODULE_H
