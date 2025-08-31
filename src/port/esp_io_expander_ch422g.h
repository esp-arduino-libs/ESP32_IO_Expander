/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_io_expander.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_IO_EXPANDER_CH422G_VER_MAJOR    (2)
#define ESP_IO_EXPANDER_CH422G_VER_MINOR    (0)
#define ESP_IO_EXPANDER_CH422G_VER_PATCH    (0)

/**
 * @brief Create a CH422G IO expander object
 *
 * @param[in]  i2c_bus    I2C bus handle. Obtained from `i2c_new_master_bus()`
 * @param[in]  i2c_dev_cfg   I2C device configuration.
 * @param[out] handle_ret Handle to created IO expander object
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
esp_err_t esp_io_expander_new_i2c_ch422g(
    i2c_master_bus_handle_t i2c_bus, const i2c_device_config_t *i2c_dev_cfg, esp_io_expander_handle_t *handle_ret
);

/**
 * @brief I2C address of the ch422g. Just to keep the same with other IO expanders, but it is ignored.
 */
#define ESP_IO_EXPANDER_I2C_CH422G_ADDRESS    (0x24)

/**
 * @brief Macro to create a I2C device configuration for CH422G
 *
 * @param[in] address I2C device address
 * @param[in] freq_hz I2C bus frequency
 * @return I2C device configuration
 */
#define ESP_IO_EXPANDER_I2C_CH422G_DEVICE_CFG(address, freq_hz) \
    { \
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, \
        .device_address = address, \
        .scl_speed_hz = freq_hz, \
    }

/**
 * @brief Set CH422G to open-drain output mode
 *
 * @param[in] handle IO expander handle
 * @return
 *      - ESP_OK: Success
 *      - Others: Failure
 */
esp_err_t esp_io_expander_ch422g_set_oc_open_drain(esp_io_expander_handle_t handle);

/**
 * @brief Set CH422G to push-pull output mode
 *
 * @param[in] handle IO expander handle
 * @return
 *      - ESP_OK: Success
 *      - Others: Failure
 */
esp_err_t esp_io_expander_ch422g_set_oc_push_pull(esp_io_expander_handle_t handle);

/**
 * @brief Set all CH422G pins to input mode
 *
 * @param[in] handle IO expander handle
 * @return
 *      - ESP_OK: Success
 *      - Others: Failure
 */
esp_err_t esp_io_expander_ch422g_set_all_input(esp_io_expander_handle_t handle);

/**
 * @brief Set all CH422G pins to output mode
 *
 * @param[in] handle IO expander handle
 * @return
 *      - ESP_OK: Success
 *      - Others: Failure
 */
esp_err_t esp_io_expander_ch422g_set_all_output(esp_io_expander_handle_t handle);

/**
 * @brief Make CH422G enter sleep mode
 *
 * @param[in] handle IO expander handle
 * @return
 *      - ESP_OK: Success
 *      - Others: Failure
 */
esp_err_t esp_io_expander_ch422g_enter_sleep(esp_io_expander_handle_t handle);

/**
 * @brief Make CH422G exit sleep mode
 *
 * @param[in] handle IO expander handle
 * @return
 *      - ESP_OK: Success
 *      - Others: Failure
 */
esp_err_t esp_io_expander_ch422g_exit_sleep(esp_io_expander_handle_t handle);

#ifdef __cplusplus
}
#endif
