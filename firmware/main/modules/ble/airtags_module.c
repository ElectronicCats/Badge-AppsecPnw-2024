#include "modules/ble/airtags_module.h"
// #include "freertos/FreeRTOS.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "inttypes.h"

// static const char remote_device_name[] = "ESP_GATTS_DEMO";
static bool connect = false;
static bool get_server = false;
static bool scan_active = false;
static esp_gattc_char_elem_t* char_elem_result = NULL;
static esp_gattc_descr_elem_t* descr_elem_result = NULL;
static uint16_t devices_found_count = 0;
static uint8_t scan_timer = 0;
static bluetooth_scanner_cb_t scanner_cb = NULL;
TaskHandle_t scan_timer_task_handle = NULL;

bool tag_finded = false;

bluetooth_scanner_record_t record = {
    .mac_address = {0},
    .rssi = 0,
    .name = "",
    .is_airtag = false,
    .count = 0,
    .has_finished = true,
};

static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid =
        {
            .uuid16 = AIRTAG_REMOTE_SERVICE_UUID,
        },
};

static esp_bt_uuid_t remote_filter_char_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid =
        {
            .uuid16 = AIRTAG_REMOTE_NOTIFY_CHAR_UUID,
        },
};

static esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid =
        {
            .uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,
        },
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x003,
    .scan_window = 0x003,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

/* One gatt-based profile one app_id and one gattc_if, this array will store the
 * gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[AIRTAG_PROFILE_NUM] = {
    [AIRTAG_PROFILE_A_APP_ID] =
        {
            .gattc_cb = gattc_profile_event_handler,
            .gattc_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is
                                             ESP_GATT_IF_NONE */
        },
};

void timer_task(void* arg) {
  scan_timer = 0;

  while (true) {
    scan_timer++;

    if (scan_timer >= AIRTAG_SCAN_DURATION) {
      bluetooth_scanner_stop();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void gattc_profile_event_handler(esp_gattc_cb_event_t event,
                                 esp_gatt_if_t gattc_if,
                                 esp_ble_gattc_cb_param_t* param) {
  esp_ble_gattc_cb_param_t* p_data = (esp_ble_gattc_cb_param_t*) param;

  switch (event) {
    case ESP_GATTC_REG_EVT:
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "REG_EVT");
      esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
      if (scan_ret) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE,
                 "set scan params error, error code = %x", scan_ret);
      }
      break;
    case ESP_GATTC_CONNECT_EVT: {
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "ESP_GATTC_CONNECT_EVT conn_id %d, if %d",
               p_data->connect.conn_id, gattc_if);
      gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
      memcpy(gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].remote_bda,
             p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "REMOTE BDA:");
      esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
                         gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].remote_bda,
                         sizeof(esp_bd_addr_t));
      esp_err_t mtu_ret =
          esp_ble_gattc_send_mtu_req(gattc_if, p_data->connect.conn_id);
      if (mtu_ret) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "config MTU error, error code = %x",
                 mtu_ret);
      }
      break;
    }
    case ESP_GATTC_OPEN_EVT:
      if (param->open.status != ESP_GATT_OK) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "open failed, status %d",
                 p_data->open.status);
        break;
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "open success");
      break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
      if (param->dis_srvc_cmpl.status != ESP_GATT_OK) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "discover service failed, status %d",
                 param->dis_srvc_cmpl.status);
        break;
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "discover service complete conn_id %d",
               param->dis_srvc_cmpl.conn_id);
      esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id,
                                   &remote_filter_service_uuid);
      break;
    case ESP_GATTC_CFG_MTU_EVT:
      if (param->cfg_mtu.status != ESP_GATT_OK) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "config mtu failed, error status = %x",
                 param->cfg_mtu.status);
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE,
               "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d",
               param->cfg_mtu.status, param->cfg_mtu.mtu,
               param->cfg_mtu.conn_id);
      break;
    case ESP_GATTC_SEARCH_RES_EVT: {
      ESP_LOGI(TAG_BLE_CLIENT_MODULE,
               "SEARCH RES: conn_id = %x is primary service %d",
               p_data->search_res.conn_id, p_data->search_res.is_primary);
      ESP_LOGI(TAG_BLE_CLIENT_MODULE,
               "start handle %d end handle %d current handle value %d",
               p_data->search_res.start_handle, p_data->search_res.end_handle,
               p_data->search_res.srvc_id.inst_id);
      if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 &&
          p_data->search_res.srvc_id.uuid.uuid.uuid16 ==
              AIRTAG_REMOTE_SERVICE_UUID) {
        ESP_LOGI(TAG_BLE_CLIENT_MODULE, "service found");
        get_server = true;
        gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_start_handle =
            p_data->search_res.start_handle;
        gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_end_handle =
            p_data->search_res.end_handle;
        ESP_LOGI(TAG_BLE_CLIENT_MODULE, "UUID16: %x",
                 p_data->search_res.srvc_id.uuid.uuid.uuid16);
      }
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
      if (p_data->search_cmpl.status != ESP_GATT_OK) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE,
                 "search service failed, error status = %x",
                 p_data->search_cmpl.status);
        break;
      }
      if (p_data->search_cmpl.searched_service_source ==
          ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
        ESP_LOGI(TAG_BLE_CLIENT_MODULE,
                 "Get service information from remote device");
      } else if (p_data->search_cmpl.searched_service_source ==
                 ESP_GATT_SERVICE_FROM_NVS_FLASH) {
        ESP_LOGI(TAG_BLE_CLIENT_MODULE, "Get service information from flash");
      } else {
        ESP_LOGI(TAG_BLE_CLIENT_MODULE, "unknown service source");
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "ESP_GATTC_SEARCH_CMPL_EVT");
      if (get_server) {
        uint16_t count = 0;
        esp_gatt_status_t status = esp_ble_gattc_get_attr_count(
            gattc_if, p_data->search_cmpl.conn_id, ESP_GATT_DB_CHARACTERISTIC,
            gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_start_handle,
            gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_end_handle,
            AIRTAG_INVALID_HANDLE, &count);
        if (status != ESP_GATT_OK) {
          ESP_LOGE(TAG_BLE_CLIENT_MODULE, "esp_ble_gattc_get_attr_count error");
          break;
        }

        if (count > 0) {
          char_elem_result = (esp_gattc_char_elem_t*) malloc(
              sizeof(esp_gattc_char_elem_t) * count);
          if (!char_elem_result) {
            ESP_LOGE(TAG_BLE_CLIENT_MODULE, "gattc no mem");
            break;
          } else {
            status = esp_ble_gattc_get_char_by_uuid(
                gattc_if, p_data->search_cmpl.conn_id,
                gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_start_handle,
                gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_end_handle,
                remote_filter_char_uuid, char_elem_result, &count);
            if (status != ESP_GATT_OK) {
              ESP_LOGE(TAG_BLE_CLIENT_MODULE,
                       "esp_ble_gattc_get_char_by_uuid error");
              free(char_elem_result);
              char_elem_result = NULL;
              break;
            }

            /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo,
             * so we used first 'char_elem_result' */
            if (count > 0 && (char_elem_result[0].properties &
                              ESP_GATT_CHAR_PROP_BIT_NOTIFY)) {
              gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].char_handle =
                  char_elem_result[0].char_handle;
              esp_ble_gattc_register_for_notify(
                  gattc_if, gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].remote_bda,
                  char_elem_result[0].char_handle);
            }
          }
          /* free char_elem_result */
          free(char_elem_result);
        } else {
          ESP_LOGE(TAG_BLE_CLIENT_MODULE, "no char found");
        }
      }
      break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
      if (p_data->reg_for_notify.status != ESP_GATT_OK) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE,
                 "REG FOR NOTIFY failed: error status = %d",
                 p_data->reg_for_notify.status);
      } else {
        uint16_t count = 0;
        uint16_t notify_en = 1;
        esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count(
            gattc_if, gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].conn_id,
            ESP_GATT_DB_DESCRIPTOR,
            gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_start_handle,
            gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].service_end_handle,
            gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].char_handle, &count);
        if (ret_status != ESP_GATT_OK) {
          ESP_LOGE(TAG_BLE_CLIENT_MODULE, "esp_ble_gattc_get_attr_count error");
          break;
        }
        if (count > 0) {
          descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
          if (!descr_elem_result) {
            ESP_LOGE(TAG_BLE_CLIENT_MODULE, "malloc error, gattc no mem");
            break;
          } else {
            ret_status = esp_ble_gattc_get_descr_by_char_handle(
                gattc_if, gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].conn_id,
                p_data->reg_for_notify.handle, notify_descr_uuid,
                descr_elem_result, &count);
            if (ret_status != ESP_GATT_OK) {
              ESP_LOGE(TAG_BLE_CLIENT_MODULE,
                       "esp_ble_gattc_get_descr_by_char_handle error");
              free(descr_elem_result);
              descr_elem_result = NULL;
              break;
            }
            /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo,
             * so we used first 'descr_elem_result' */
            if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 &&
                descr_elem_result[0].uuid.uuid.uuid16 ==
                    ESP_GATT_UUID_CHAR_CLIENT_CONFIG) {
              ret_status = esp_ble_gattc_write_char_descr(
                  gattc_if, gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].conn_id,
                  descr_elem_result[0].handle, sizeof(notify_en),
                  (uint8_t*) &notify_en, ESP_GATT_WRITE_TYPE_RSP,
                  ESP_GATT_AUTH_REQ_NONE);
            }

            if (ret_status != ESP_GATT_OK) {
              ESP_LOGE(TAG_BLE_CLIENT_MODULE,
                       "esp_ble_gattc_write_char_descr error");
            }

            /* free descr_elem_result */
            free(descr_elem_result);
          }
        } else {
          ESP_LOGE(TAG_BLE_CLIENT_MODULE, "decsr not found");
        }
      }
      break;
    }
    case ESP_GATTC_NOTIFY_EVT:
      if (p_data->notify.is_notify) {
        ESP_LOGI(TAG_BLE_CLIENT_MODULE,
                 "ESP_GATTC_NOTIFY_EVT, receive notify value:");
      } else {
        ESP_LOGI(TAG_BLE_CLIENT_MODULE,
                 "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
      }
      esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE, p_data->notify.value,
                         p_data->notify.value_len);
      break;
    case ESP_GATTC_WRITE_DESCR_EVT:
      if (p_data->write.status != ESP_GATT_OK) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "write descr failed, error status = %x",
                 p_data->write.status);
        break;
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "write descr success ");
      uint8_t write_char_data[35];
      for (int i = 0; i < sizeof(write_char_data); ++i) {
        write_char_data[i] = i % 256;
      }
      esp_ble_gattc_write_char(
          gattc_if, gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].conn_id,
          gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].char_handle,
          sizeof(write_char_data), write_char_data, ESP_GATT_WRITE_TYPE_RSP,
          ESP_GATT_AUTH_REQ_NONE);
      break;
    case ESP_GATTC_SRVC_CHG_EVT: {
      esp_bd_addr_t bda;
      memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
      esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE, bda, sizeof(esp_bd_addr_t));
      break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
      if (p_data->write.status != ESP_GATT_OK) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "write char failed, error status = %x",
                 p_data->write.status);
        break;
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "write char success ");
      break;
    case ESP_GATTC_DISCONNECT_EVT:
      connect = false;
      get_server = false;
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "ESP_GATTC_DISCONNECT_EVT, reason = %d",
               p_data->disconnect.reason);
      break;
    default:
      break;
  }
}

void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  uint8_t* adv_name = NULL;
  uint8_t adv_name_len = 0;
  switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "Start scanning...");
      // the unit of the duration is second
      uint32_t duration = AIRTAG_SCAN_DURATION;
      esp_ble_gap_start_scanning(duration);
      break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
      // scan start complete event to indicate scan start successfully or failed
      if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "scan start failed, error status = %x",
                 param->scan_start_cmpl.status);
        break;
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "scan start success");

      break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
      esp_ble_gap_cb_param_t* scan_result = (esp_ble_gap_cb_param_t*) param;
      switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
          if (!scan_active) {
            break;
          }
          if (scan_result->scan_rst.adv_data_len > 0) {
            // ESP_LOGI(TAG_BLE_CLIENT_MODULE, "Address:
            // %02X:%02X:%02X:%02X:%02X:%02X",
            //         scan_result->scan_rst.bda[5],
            //         scan_result->scan_rst.bda[4],
            //         scan_result->scan_rst.bda[3],
            //         scan_result->scan_rst.bda[2],
            //         scan_result->scan_rst.bda[1],
            //         scan_result->scan_rst.bda[0]);
            // // esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
            // scan_result->scan_rst.bda, 6); ESP_LOGI(TAG_BLE_CLIENT_MODULE,
            // "searched Adv Data Len %d, Scan Response Len %d",
            //         scan_result->scan_rst.adv_data_len,
            //         scan_result->scan_rst.scan_rsp_len);
            // adv_name =
            // esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
            //                                     ESP_BLE_AD_TYPE_NAME_CMPL,
            //                                     &adv_name_len);
            // ESP_LOGI(TAG_BLE_CLIENT_MODULE, "searched Device Name Len %d",
            // adv_name_len); esp_log_buffer_char(TAG_BLE_CLIENT_MODULE,
            // adv_name, adv_name_len); ESP_LOGI(TAG_BLE_CLIENT_MODULE, " ");
            if (scan_result->scan_rst.bda[0] == 0xcf) {
              ESP_LOGI(TAG_BLE_CLIENT_MODULE, "Tile----------------");
              ESP_LOGI(
                  TAG_BLE_CLIENT_MODULE,
                  "Address: %02X:%02X:%02X:%02X:%02X:%02X",
                  scan_result->scan_rst.bda[0], scan_result->scan_rst.bda[1],
                  scan_result->scan_rst.bda[2], scan_result->scan_rst.bda[3],
                  scan_result->scan_rst.bda[4], scan_result->scan_rst.bda[5]);
              ESP_LOGI(TAG_BLE_CLIENT_MODULE,
                       "searched Adv Data Len %d, Scan Response Len %d",
                       scan_result->scan_rst.adv_data_len,
                       scan_result->scan_rst.scan_rsp_len);
              adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                  ESP_BLE_AD_TYPE_NAME_CMPL,
                                                  &adv_name_len);
              ESP_LOGI(TAG_BLE_CLIENT_MODULE, "searched Device Name Len %d",
                       adv_name_len);
              esp_log_buffer_char(TAG_BLE_CLIENT_MODULE, adv_name,
                                  adv_name_len);
              esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
                                 &scan_result->scan_rst.ble_adv[0],
                                 scan_result->scan_rst.adv_data_len);
              esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
                                 &scan_result->scan_rst.ble_adv[1],
                                 scan_result->scan_rst.adv_data_len);
              esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
                                 &scan_result->scan_rst.ble_adv[2],
                                 scan_result->scan_rst.adv_data_len);
              esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
                                 &scan_result->scan_rst.ble_adv[3],
                                 scan_result->scan_rst.adv_data_len);
            }
            // https://adamcatley.com/AirTag
            //  REGISTERED
            if (scan_result->scan_rst.ble_adv[0] == 0x1E &&
                scan_result->scan_rst.ble_adv[1] == 0xFF &&
                scan_result->scan_rst.ble_adv[2] == 0x4C &&
                scan_result->scan_rst.ble_adv[3] == 0x00) {
              tag_finded = true;
              record.name = "ATag";
            }
            // UNREGISTERED
            if (scan_result->scan_rst.ble_adv[0] == 0x4C &&
                scan_result->scan_rst.ble_adv[1] == 0x00 &&
                scan_result->scan_rst.ble_adv[2] == 0x12 &&
                scan_result->scan_rst.ble_adv[3] == 0x19) {
              tag_finded = true;
              record.name = "UATag";
            }
            if (scan_result->scan_rst.ble_adv[0] == 0x02 &&
                scan_result->scan_rst.ble_adv[1] == 0x01 &&
                scan_result->scan_rst.ble_adv[2] == 0x06 &&
                scan_result->scan_rst.ble_adv[3] == 0x0D) {
              ESP_LOGI(TAG_BLE_CLIENT_MODULE, "Tile==========");
              tag_finded = true;
              record.name = "Tile";
            }

            if (tag_finded) {
              memcpy(record.mac_address, scan_result->scan_rst.bda, 6);
              record.rssi = scan_result->scan_rst.rssi;
              // record.name         = "Unknown";
              record.is_airtag = false;
              record.count = devices_found_count++;
              record.has_finished = false;

              ESP_LOGI(
                  TAG_BLE_CLIENT_MODULE,
                  "Address: %02X:%02X:%02X:%02X:%02X:%02X",
                  scan_result->scan_rst.bda[5], scan_result->scan_rst.bda[4],
                  scan_result->scan_rst.bda[3], scan_result->scan_rst.bda[2],
                  scan_result->scan_rst.bda[1], scan_result->scan_rst.bda[0]);
              // esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
              // scan_result->scan_rst.bda, 6);
              ESP_LOGI(TAG_BLE_CLIENT_MODULE,
                       "searched Adv Data Len %d, Scan Response Len %d",
                       scan_result->scan_rst.adv_data_len,
                       scan_result->scan_rst.scan_rsp_len);
              adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                  ESP_BLE_AD_TYPE_NAME_CMPL,
                                                  &adv_name_len);
              ESP_LOGI(TAG_BLE_CLIENT_MODULE, "searched Device Name Len %d",
                       adv_name_len);
              esp_log_buffer_char(TAG_BLE_CLIENT_MODULE, adv_name,
                                  adv_name_len);

              record.is_airtag = true;

              ESP_LOGI(TAG_BLE_CLIENT_MODULE, "adv data:");
              esp_log_buffer_hex(TAG_BLE_CLIENT_MODULE,
                                 &scan_result->scan_rst.ble_adv[0],
                                 scan_result->scan_rst.adv_data_len);
              esp_log_buffer_char(TAG_BLE_CLIENT_MODULE,
                                  &scan_result->scan_rst.ble_adv[0],
                                  scan_result->scan_rst.adv_data_len);
              if (scanner_cb) {
                scanner_cb(record);
              }
              ESP_LOGI(TAG_BLE_CLIENT_MODULE, " ");
            }
            tag_finded = false;
          }
          // if (scan_result->scan_rst.scan_rsp_len > 0) {
          //   ESP_LOGI(TAG_BLE_CLIENT_MODULE, "scan resp:");
          //   esp_log_buffer_hex(
          //       TAG_BLE_CLIENT_MODULE,
          //       &scan_result->scan_rst
          //            .ble_adv[scan_result->scan_rst.adv_data_len],
          //       scan_result->scan_rst.scan_rsp_len);
          // }
          // #endif

          // if (adv_name != NULL) {
          ////   if (strlen(remote_device_name) == adv_name_len &&
          ////       strncmp((char*) adv_name, remote_device_name, adv_name_len)
          ///==
          //           0) {
          ////     ESP_LOGI(TAG_BLE_CLIENT_MODULE, "searched device %s",
          /// remote_device_name);
          //     if (connect == false) {
          //       connect = true;
          //       ESP_LOGI(TAG_BLE_CLIENT_MODULE, "connect to the remote
          //       device.");
          //       //esp_ble_gap_stop_scanning();
          //       esp_ble_gattc_open(gl_profile_tab[AIRTAG_PROFILE_A_APP_ID].gattc_if,
          //                          scan_result->scan_rst.bda,
          //                          scan_result->scan_rst.ble_addr_type,
          //                          true);
          //     }
          //   }
          // }

          // if (record.is_airtag) {
          //   bluetooth_scanner_stop();
          // }
          break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
          break;
        default:
          break;
      }
      break;
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
      if (scanner_cb) {
        record.has_finished = true;
        scanner_cb(record);
        ESP_LOGI(TAG_BLE_CLIENT_MODULE, "%s", record.name);
      }

      if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "scan stop failed, error status = %x",
                 param->scan_stop_cmpl.status);
        break;
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "stop scan successfully");
      break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
      if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(TAG_BLE_CLIENT_MODULE, "adv stop failed, error status = %x",
                 param->adv_stop_cmpl.status);
        break;
      }
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "stop adv successfully");
      break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
      ESP_LOGI(
          TAG_BLE_CLIENT_MODULE,
          "update connection params status = %d, min_int = %d, max_int = "
          "%d,conn_int = %d,latency = %d, timeout = %d",
          param->update_conn_params.status, param->update_conn_params.min_int,
          param->update_conn_params.max_int, param->update_conn_params.conn_int,
          param->update_conn_params.latency, param->update_conn_params.timeout);
      break;
    default:
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "event %d", event);
      break;
  }
}

void esp_gattc_cb(esp_gattc_cb_event_t event,
                  esp_gatt_if_t gattc_if,
                  esp_ble_gattc_cb_param_t* param) {
  /* If event is register event, store the gattc_if for each profile */
  if (event == ESP_GATTC_REG_EVT) {
    if (param->reg.status == ESP_GATT_OK) {
      gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
    } else {
      ESP_LOGI(TAG_BLE_CLIENT_MODULE, "reg app failed, app_id %04x, status %d",
               param->reg.app_id, param->reg.status);
      return;
    }
  }

  /* If the gattc_if equal to profile A, call profile A cb handler,
   * so here call each profile's callback */
  do {
    int idx;
    for (idx = 0; idx < AIRTAG_PROFILE_NUM; idx++) {
      if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a
                                             certain gatt_if, need to call every
                                             profile cb function */
          gattc_if == gl_profile_tab[idx].gattc_if) {
        if (gl_profile_tab[idx].gattc_cb) {
          gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
        }
      }
    }
  } while (0);
}

void bluetooth_scanner_init() {
  // Initialize NVS.
  esp_err_t ret;

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE, "%s initialize controller failed: %s",
             __func__, esp_err_to_name(ret));
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE, "%s enable controller failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
  ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg);
  if (ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE, "%s init bluetooth failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  ret = esp_bluedroid_enable();
  if (ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE, "%s enable bluetooth failed: %s", __func__,
             esp_err_to_name(ret));
    return;
  }

  // register the  callback function to the gap module
  ret = esp_ble_gap_register_callback(esp_gap_cb);
  if (ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE, "%s gap register failed, error code = %x",
             __func__, ret);
    return;
  }

  // register the callback function to the gattc module
  ret = esp_ble_gattc_register_callback(esp_gattc_cb);
  if (ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE, "%s gattc register failed, error code = %x",
             __func__, ret);
    return;
  }

  ret = esp_ble_gattc_app_register(AIRTAG_PROFILE_A_APP_ID);
  if (ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE,
             "%s gattc app register failed, error code = %x", __func__, ret);
  }
  esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
  if (local_mtu_ret) {
    ESP_LOGE(TAG_BLE_CLIENT_MODULE, "set local  MTU failed, error code = %x",
             local_mtu_ret);
  }

  xTaskCreate(timer_task, "timer_task", 2048, NULL, 10,
              &scan_timer_task_handle);
  vTaskSuspend(scan_timer_task_handle);
  bluetooth_scanner_stop();
}

void bluetooth_scanner_register_cb(bluetooth_scanner_cb_t callback) {
  scanner_cb = callback;
}

void bluetooth_scanner_start() {
  ESP_LOGI(TAG_BLE_CLIENT_MODULE, "Starting Bluetooth scanner");
  scan_active = true;
  devices_found_count = 0;
  esp_ble_gap_set_scan_params(&ble_scan_params);
  scan_timer = 0;
  vTaskResume(scan_timer_task_handle);
}

void bluetooth_scanner_stop() {
  scan_active = false;
  esp_ble_gap_stop_scanning();
  vTaskSuspend(scan_timer_task_handle);
  // bluetooth_scanner_deinit();
}

void bluetooth_scanner_deinit() {
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
}

bool bluetooth_scanner_is_active() {
  return scan_active;
}
