#ifndef BLE_CLIENT_MODULE_H
#define BLE_CLIENT_MODULE_H
#define TAG_BLE_CLIENT_MODULE "bt_gattc:ble_client_module"

/**
 * @brief Initialize the GATT Client profile
 *
 * @return void
 */
void ble_client_task_begin(void);
/**
 * @brief Stop the GATT Client profile
 *
 * @return void
 */
void ble_client_task_stop(void);
#endif  // BLE_CLIENT_MODULE_H
