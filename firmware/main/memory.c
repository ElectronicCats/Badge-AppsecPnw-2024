#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "memory.h"

#define TAG_NVS "EC_APPSECPWN_NVS"

esp_err_t return_err;
nvs_handle_t nvs_handler;

void init_nvs() {

    // Initialize NVS
    return_err = nvs_flash_init();
    if (return_err == ESP_ERR_NVS_NO_FREE_PAGES || return_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        return_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(return_err);

    ESP_LOGI(TAG_NVS, "Opening Non-Volatile Storage (NVS) handle...");
}

esp_err_t open_nvs() {
    // Open
    return_err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handler);
    return return_err;
}

void close_nvs() {
    nvs_close(nvs_handler);
    ESP_LOGI(TAG_NVS, "NVS handle closed...");
}

esp_err_t write_int32_nvs(int32_t *value) {
    //ESP_LOGI(TAG_NVS, "Setting %s to %d", key, value);
    return_err = open_nvs();
    if (return_err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error opening NVS handle...");
        return return_err;
    }

    return_err = nvs_set_i32(nvs_handler, "storage", &value);
    ESP_ERROR_CHECK(return_err);
    ESP_LOGI(TAG_NVS, "Committing updates in NVS...");
    return_err = nvs_commit(nvs_handler);
    
    if (return_err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) writing!", esp_err_to_name(return_err));
    }

    //ESP_LOGI(TAG_NVS, "Write %"PRIu32 "\n", value);
    close_nvs();
    return return_err;
}

esp_err_t read_int32_nvs(int32_t *buffer_int32) {
    return_err = open_nvs();
    if (return_err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error opening NVS handle...");
        return return_err;
    }

    return_err = nvs_get_i32(nvs_handler, "storage", &buffer_int32);
    if (return_err != ESP_OK) {
        ESP_LOGE(TAG_NVS, "Error (%s) reading!", esp_err_to_name(return_err));
    }
    close_nvs();
    return return_err;
}   