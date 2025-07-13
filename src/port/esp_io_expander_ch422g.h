/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
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
 * @param[in]  dev_addr   I2C device address of chip. Can be `ESP_IO_EXPANDER_I2C_CH422G_ADDRESS`.
 * @param[out] handle_ret Handle to created IO expander object
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
esp_err_t esp_io_expander_new_i2c_ch422g(i2c_master_bus_handle_t i2c_bus, uint32_t dev_addr, esp_io_expander_handle_t *handle_ret);

/**
 * @brief I2C address of the ch422g. Just to keep the same with other IO expanders, but it is ignored.
 */
#define ESP_IO_EXPANDER_I2C_CH422G_ADDRESS    (0x24)

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
