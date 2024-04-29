#ifndef GAME_BLE_SERVER_H
#define GAME_BLE_SERVER_H
#define TAG_BLE_SERVER_MODULE "bt_gatts:game_ble_server"

/**
 * @brief Start the GATT Server profile
 *
 * @return void
 */
void ble_server_task_begin(void);
/**
 * @brief Stop the GATT Server profile
 *
 * @return void
 */
void ble_server_task_stop(void);
#endif  // GAME_BLE_SERVER_H
