#include "esp_err.h"
#ifndef MEMORY_H
  #define MEMORY_H

  #define STORAGE_NAMESPACE "storage"

/**
 * @brief Initialize the non-volatile storage (NVS) partition
 *
 * @return void
 */
void memory_driver_begin();

/**
 * @brief Open the non-volatile storage (NVS) partition
 *
 * @return esp_err_t The error code
 */
esp_err_t memory_driver_open_nvs();

/**
 * @brief Close the non-volatile storage (NVS) partition
 *
 * @return void
 */
void memory_driver_close_nvs();

/**
 * @brief Write an int32_t value to the non-volatile storage (NVS) partition
 *
 * @param key The key to write the value to
 * @param value The value to write
 * @return esp_err_t The error code
 */
esp_err_t memory_driver_write_int32_nvs(int32_t* value);

/**
 * @brief Read an int32_t value from the non-volatile storage (NVS) partition
 *
 * @param key The key to read the value from
 * @param value The pointer to the int32_t value to read
 * @return esp_err_t The error code
 */
esp_err_t memory_driver_read_int32_nvs(int32_t* buffer_int32);
#endif  // MEMORY_H
