#ifndef GAME_BLE_CLIENT_H
#define GAME_BLE_CLIENT_H
#define TAG_BLE_CLIENT_MODULE "bt_gattc:game_ble_client"

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
#endif  // GAME_BLE_CLIENT_H
