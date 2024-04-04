#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_vfs_eventfd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "ble_profiles.h"
#include "display.h"
#include "engine.h"

static uint8_t adv_config_done = 0;
static uint8_t test_manufacturer[BOARD_MANUFACTURER_DATA_LEN] = {0x12, 0x23, 0x45, 0x56};
static uint8_t char1_str[] = {0x11, 0x22, 0x33};
static esp_gatt_char_prop_t username_property = 0;
static prepare_type_env_t a_prepare_write_env;

bool ble_running_service = false;

static esp_attr_value_t gatts_demo_char1_val =
    {
        .attr_max_len = GATTS_DEVICE_CHAR_VAL_LEN_MAX,
        .attr_len = sizeof(char1_str),
        .attr_value = char1_str,
};

PlayerInformation red_player_information = {
    .username = "RED0000",
    .life_points = GAME_DEFAULT_LIFE_POINTS,
    .team = RED
};

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xEE,
    0x00,
    0x00,
    0x00,
    // second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};
// ADV Data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,  // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010,  // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,        // BOARD_MANUFACTURER_DATA_LEN, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL,  //&test_manufacturer[0], //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// Scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00,
    .manufacturer_len = BOARD_MANUFACTURER_DATA_LEN,  // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = &test_manufacturer[0],     //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};


void get_player_array(esp_gatt_rsp_t* response_information, PlayerInformation player) {
    for (int i = 0; i < sizeof(player.username); i++) {
        response_information->attr_value.value[i] = player.username[i];
    }
}

void get_profile_owasp_array(esp_gatt_rsp_t* response_information, OWASPProfile *profile) {
    for (int i = 0; i < sizeof(profile->vuln->cwe); i++) {
        response_information->attr_value.value[i] = profile->vuln->cwe[i];
    }
}

void get_profile_owasp_uint8(uint8_t* response_information, OWASPProfile *profile) {
    for (int i = 0; i < sizeof(profile->vuln->cwe); i++) {
        response_information[i] = profile->vuln->cwe[i];
    }
}

void get_player_array_uint8(uint8_t* response_information, PlayerInformation player) {
    for (int i = 0; i < sizeof(player.username); i++) {
        response_information[i] = player.username[i];
    }
}

void send_ble_data(uint8_t* data, int length) {
    esp_ble_gatts_send_indicate(gatts_profile_tab[DEVICE_PROFILE].gatts_if,
                                gatts_profile_tab[DEVICE_PROFILE].conn_id,
                                gatts_profile_tab[DEVICE_PROFILE].char_handle,
                                length, data, true);
}

// ADV Event Handler
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~adv_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~scan_rsp_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            // advertising start complete event to indicate advertising start successfully or failed
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG_BLE, "Advertising start failed\n");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG_BLE, "Advertising stop failed\n");
            } else {
                ESP_LOGI(TAG_BLE, "Stop ADV successfully\n");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(TAG_BLE, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                     param->update_conn_params.status,
                     param->update_conn_params.min_int,
                     param->update_conn_params.max_int,
                     param->update_conn_params.conn_int,
                     param->update_conn_params.latency,
                     param->update_conn_params.timeout);
            break;
        default:
            break;
    }
}

void board_write_event(esp_gatt_if_t gatts_if, prepare_type_env_t* prepare_write_env, esp_ble_gatts_cb_param_t* param) {
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp) {
        if (param->write.is_prep) {
            if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
                ESP_LOGW(TAG_BLE, "invalid prepare write, offset = %d, value len = %d",
                         param->write.offset, param->write.len);
                status = ESP_GATT_INVALID_OFFSET;
            } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                ESP_LOGW(TAG_BLE, "invalid prepare write, invalid length, offset = %d, value len = %d",
                         param->write.offset, param->write.len);
                status = ESP_GATT_INVALID_ATTR_LEN;
            }

            if (status == ESP_GATT_OK && prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t*)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;

                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(TAG_BLE, "Gatt_server prep no mem");
                    status = ESP_GATT_NO_RESOURCES;
                }
            }

            esp_gatt_rsp_t* gatt_rsp = (esp_gatt_rsp_t*)malloc(sizeof(esp_gatt_rsp_t));
            if (gatt_rsp) {
                gatt_rsp->attr_value.len = param->write.len;
                gatt_rsp->attr_value.handle = param->write.handle;
                gatt_rsp->attr_value.offset = param->write.offset;
                gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
                memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);

                esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
                if (response_err != ESP_OK) {
                    ESP_LOGE(TAG_BLE, "Send response error");
                }
                free(gatt_rsp);
            } else {
                ESP_LOGE(TAG_BLE, "Gatt_server malloc failled, no resources");
                status = ESP_GATT_NO_RESOURCES;
            }

            if (status != ESP_GATT_OK) {
                return;
            }

            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;

        } else {
            ESP_LOGI(TAG_BLE, "board_write_event, Not prep");
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

void board_exce_write_event(prepare_type_env_t* prepare_write_env, esp_ble_gatts_cb_param_t* param) {
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC) {
        esp_log_buffer_hex(TAG_BLE, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
        esp_log_buffer_char(TAG_BLE, param->write.value, param->write.len);
    } else {
        ESP_LOGI(TAG_BLE, "ESP_GATT_PREP_WRITE_CANCEL");
    }

    // Clean up prepare buffer

    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

void gatts_profiles_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(TAG_BLE, "REGISTER_APP_EVT, status %d, app_id %d\n",
                     param->reg.status, param->reg.app_id);
            gatts_profile_tab[DEVICE_PROFILE].service_id.is_primary = true;
            gatts_profile_tab[DEVICE_PROFILE].service_id.id.inst_id = 0x00;
            gatts_profile_tab[DEVICE_PROFILE].service_id.id.uuid.len = ESP_UUID_LEN_16;
            gatts_profile_tab[DEVICE_PROFILE].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_USERNAME;

            esp_err_t set_device_name = esp_ble_gap_set_device_name(BOARD_RED_DEVICE_NAME);
            if (set_device_name) {
                ESP_LOGE(TAG_BLE, "Set device name failed, error code = %x", set_device_name);
            }

            // Configure ADV data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret) {
                ESP_LOGE(TAG_BLE, "Config ADV failed, error code: %x", ret);
            }

            adv_config_done |= adv_config_flag;
            // Config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret) {
                ESP_LOGE(TAG_BLE, "Config scan response data failed, error code: %x", ret);
            }

            adv_config_done |= scan_rsp_config_flag;

            esp_ble_gatts_create_service(gatts_if, &gatts_profile_tab[DEVICE_PROFILE].service_id, GATTS_NUM_HANDLE_USERNAME);
            break;
        case ESP_GATTS_READ_EVT: {
            ESP_LOGI(TAG_BLE, "GATT_READ_EVT, conn_id %d, trans_id %" PRIu32 ", handle %d",
                     param->read.conn_id, param->read.trans_id, param->read.handle);
            esp_gatt_rsp_t response;
            memset(&response, 0, sizeof(esp_gatt_rsp_t));
            response.attr_value.handle = param->read.handle;
            response.attr_value.len = USERNAME_LENGTH;

            get_player_array(&response, red_player_information);

            // response.attr_value.value = player_array_username;
            esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                        ESP_GATT_OK, &response);

            break;
        }
        case ESP_GATTS_WRITE_EVT:{
            ESP_LOGI(TAG_BLE, "GATT_WRITE_EVT, conn_id %d, trans_id %" PRIu32 ", handle %d",
                     param->write.conn_id, param->write.trans_id, param->write.handle);
            if (!param->write.is_prep){
                ESP_LOGI(TAG_BLE, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
                esp_log_buffer_hex(TAG_BLE, param->write.value, param->write.len);
                esp_log_buffer_char(TAG_BLE, param->write.value, param->write.len);
                if (gatts_profile_tab[DEVICE_PROFILE].descr_handle == param->write.handle && param->write.len == 2){
                    uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                    if (descr_value == 0x0001){
                        if (username_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                            ESP_LOGI(TAG_BLE, "notify enable");
                            uint8_t notify_data[sizeof(red_player_information.username)];
                            for(int i = 0; i < sizeof(red_player_information.username); i++){
                                notify_data[i] = red_player_information.username[i];
                            }
                            //the size of notify_data[] need less than MTU size
                            ESP_LOGI(TAG_BLE, "SENDING USERNAME");
                            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gatts_profile_tab[DEVICE_PROFILE].char_handle,
                                                    sizeof(notify_data), notify_data, false);
                        }
                    }else if (descr_value == 0x0002){
                        if (username_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                            ESP_LOGI(TAG_BLE, "indicate enable");
                            uint8_t indicate_data[15];
                            for (int i = 0; i < sizeof(indicate_data); ++i)
                            {
                                indicate_data[i] = i%0xff;
                            }
                            //the size of indicate_data[] need less than MTU size
                            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gatts_profile_tab[DEVICE_PROFILE].char_handle,
                                                    sizeof(indicate_data), indicate_data, true);
                        }
                    }
                    else if (descr_value == 0x0000){
                        ESP_LOGI(TAG_BLE, "notify/indicate disable ");
                    }else{
                        ESP_LOGE(TAG_BLE, "unknown descr value");
                        esp_log_buffer_hex(TAG_BLE, param->write.value, param->write.len);
                    }

                }
            }
            board_write_event(gatts_if, &a_prepare_write_env, param);
            break;
        }
        case ESP_GATTS_EXEC_WRITE_EVT:
            ESP_LOGI(TAG_BLE, "ESP_GATTS_EXEC_WRITE_EVT");
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            board_exce_write_event(&a_prepare_write_env, param);
            break;
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(TAG_BLE, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            break;
        case ESP_GATTS_UNREG_EVT:
            break;
        case ESP_GATTS_CREATE_EVT: {
            ESP_LOGI(TAG_BLE, "CREATE_SERVICE_EVT, status %d,  service_handle %d",
                     param->create.status, param->create.service_handle);
            gatts_profile_tab[DEVICE_PROFILE].service_handle = param->create.service_handle;
            gatts_profile_tab[DEVICE_PROFILE].char_uuid.len = ESP_UUID_LEN_16;
            gatts_profile_tab[DEVICE_PROFILE].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_USERNAME;
            gatts_profile_tab[DEVICE_PROFILE].descr_uuid.len = ESP_UUID_LEN_16;
            gatts_profile_tab[DEVICE_PROFILE].descr_uuid.uuid.uuid16 = GATTS_DESCR_UUID_USERNAME;

            esp_ble_gatts_start_service(gatts_profile_tab[DEVICE_PROFILE].service_handle);
            username_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
            //username_property = ESP_GATT_CHAR_PROP_BIT_READ;
            esp_err_t add_chart_ret = esp_ble_gatts_add_char(gatts_profile_tab[DEVICE_PROFILE].service_handle,
                                                             &gatts_profile_tab[DEVICE_PROFILE].char_uuid,
                                                             ESP_GATT_PERM_READ,
                                                             username_property,
                                                             &gatts_demo_char1_val, NULL);
            if (add_chart_ret) {
                ESP_LOGE(TAG_BLE, "add char failed, error code =%x", add_chart_ret);
            }
            break;
        }
        case ESP_GATTS_ADD_INCL_SRVC_EVT:
            break;
        case ESP_GATTS_ADD_CHAR_EVT: {
            ESP_LOGI(TAG_BLE, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d",
                     param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

            uint16_t length = 0;
            const uint8_t* preference_char;

            gatts_profile_tab[DEVICE_PROFILE].char_handle = param->add_char.attr_handle;
            gatts_profile_tab[DEVICE_PROFILE].descr_uuid.len = ESP_UUID_LEN_16;
            gatts_profile_tab[DEVICE_PROFILE].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

            esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle, &length, &preference_char);
            if (get_attr_ret == ESP_FAIL) {
                ESP_LOGE(TAG_BLE, "ILEGAL HANDLE");
            }

            ESP_LOGI(TAG_BLE, "the gatts username char length = %d", length);
            for (int i = 0; i < length; i++) {
                ESP_LOGI(TAG_BLE, "preference_char[%d] = %x", i, preference_char[i]);
            }

            esp_err_t add_description_ret = esp_ble_gatts_add_char_descr(gatts_profile_tab[DEVICE_PROFILE].service_handle,
                                                                         &gatts_profile_tab[DEVICE_PROFILE].descr_uuid,
                                                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                                         NULL, NULL);
            if (add_description_ret) {
                ESP_LOGE(TAG_BLE, "add char descr failed, error code =%x", add_description_ret);
            }
            break;
        }
        case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
            ESP_LOGI(TAG_BLE, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d",
                     param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
            gatts_profile_tab[DEVICE_PROFILE].descr_handle = param->add_char.attr_handle;
            break;
        }
        case ESP_GATTS_DELETE_EVT:
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(TAG_BLE, "SERVICE_START_EVT, status %d, service_handle %d",
                     param->start.status, param->start.service_handle);
            break;
        case ESP_GATTS_STOP_EVT:
            break;
        case ESP_GATTS_CONNECT_EVT: {
            ESP_LOGI(TAG_BLE, "CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                     param->connect.conn_id,
                     param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                     param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);

            esp_ble_conn_update_params_t connection_params = {0};
            memcpy(connection_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));

            connection_params.latency = 0;
            connection_params.max_int = 0x20;  // max_int = 0x20*1.25ms = 40ms
            connection_params.min_int = 0x10;  // min_int = 0x10*1.25ms = 20ms
            connection_params.timeout = 400;   // timeout = 400*10ms = 4000ms

            gatts_profile_tab[DEVICE_PROFILE].conn_id = param->connect.conn_id;

            // Sent update connection parameters to the peer device
            esp_ble_gap_update_conn_params(&connection_params);

            //show_ble_connection(param->connect.remote_bda);

            ble_game_paired();
            break;
        }
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(TAG_BLE, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x",
                     param->disconnect.reason);
            esp_ble_gap_start_advertising(&adv_params);
            break;
        case ESP_GATTS_CONF_EVT: {
            ESP_LOGI(TAG_BLE, "ESP_GATTS_CONF_EVT, status %d attr_handle %d",
                     param->conf.status, param->conf.handle);
            if (param->conf.status != ESP_GATT_OK) {
                ESP_LOGW(TAG_BLE, "Confirm error, status %d", param->conf.status);
                esp_log_buffer_hex(TAG_BLE, param->conf.value, param->conf.len);
                esp_log_buffer_char(TAG_BLE, param->conf.value, param->conf.len);
            }
            break;
        }
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        default:
            break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) {
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gatts_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGI(TAG_BLE, "Register app failed, app_id %04x, status %d\n",
                     param->reg.app_id, param->reg.status);
            return;
        }
    }

    // Call the event handler for the specific profile
    do {
        int id_profile;
        for (id_profile = 0; id_profile < DEVICE_PROFILES; id_profile++) {
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gatts_profile_tab[id_profile].gatts_if) {
                if (gatts_profile_tab[id_profile].gatts_cb) {
                    gatts_profile_tab[id_profile].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}


void init_ble_task(void) {
    if (ble_running_service){
        return;
    }
    
    esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bluetooth_config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bluetooth_config);
    if (ret) {
        ESP_LOGE(TAG_BLE, "%s initialize controller failed: %s",
                 __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG_BLE, "%s enable controller failed: %s",
                 __func__, esp_err_to_name(ret));
        return;
    }

    esp_bluedroid_config_t bluedroid_config = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&bluedroid_config);
    if (ret) {
        ESP_LOGE(TAG_BLE, "%s initialize bluedroid failed: %s",
                 __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG_BLE, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret) {
        ESP_LOGE(TAG_BLE, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret) {
        ESP_LOGE(TAG_BLE, "gap register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(DEVICE_PROFILE);
    if (ret) {
        ESP_LOGE(TAG_BLE, "gatts app register error, error code = %x", ret);
        return;
    }
    
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret) {
        ESP_LOGE(TAG_BLE, "set local  MTU failed, error code = %x", local_mtu_ret);
    }

    ble_running_service = true;
}

void stop_ble_task(void) {
    ble_running_service = false;
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
}