#include "drivers/wifi_driver.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "string.h"

static bool wifi_driver_initialized = false;
static uint8_t default_ap_mac[6];

wifi_config_t wifi_driver_access_point_begin() {
  // ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_config_t wifi_manager_config = {
      .ap = {.ssid = WIFI_MANAGER_AP_SSID,
             .ssid_len = strlen(WIFI_MANAGER_AP_SSID),
             .password = WIFI_MANAGER_AP_PASSWORD,
             .max_connection = WIFI_MANAGER_AP_MAX_CONNECTIONS,
             .authmode = WIFI_AUTH_WPA2_PSK},
  };
  wifi_driver_ap_start(&wifi_manager_config);
  return wifi_manager_config;
}

void wifi_driver_ap_start(wifi_config_t* wifi_ap_config) {
  ESP_LOGI(TAG_WIFI_DRIVER, "Starting WiFi Access Point");
  if (!wifi_driver_initialized) {
    wifi_driver_init_apsta();
  }

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, wifi_ap_config));
  ESP_LOGI(TAG_WIFI_DRIVER, "WiFi Access Point started SSID: %s",
           wifi_ap_config->ap.ssid);
}

void wifi_driver_ap_stop(void) {
  ESP_LOGI(TAG_WIFI_DRIVER, "Stopping WiFi Access Point");
  wifi_config_t wifi_ap_config = {
      .ap = {.max_connection = 0},
  };
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
  ESP_LOGI(TAG_WIFI_DRIVER, "WiFi Access Point stopped");
}

void wifi_driver_init_apsta(void) {
  ESP_ERROR_CHECK(esp_netif_init());

  // Create a defailt WiFi Access Point
  esp_netif_create_default_wifi_ap();
  // Create a default WiFi Station
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  // This mode is used to connect to a WiFi network and create an Access Point
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  // Save the MAC address of the default Access Point
  ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_AP, default_ap_mac));
  ESP_ERROR_CHECK(esp_wifi_start());
  wifi_driver_initialized = true;
}
