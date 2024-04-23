#ifndef BLE_SERVER_MODULE_H
#define BLE_SERVER_MODULE_H
#define TAG_BLE_SERVER_MODULE "bt_gatts:ble_server_module"

void ble_server_task_begin(void);
/**
 * @brief Stop the GATT Server profile
 *
 * @return void
 */
void ble_server_task_stop(void);
#endif  // BLE_SERVER_MODULE_H
