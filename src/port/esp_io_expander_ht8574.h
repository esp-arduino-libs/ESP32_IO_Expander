/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
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

#define ESP_IO_EXPANDER_HT8574_VER_MAJOR    (2)
#define ESP_IO_EXPANDER_HT8574_VER_MINOR    (0)
#define ESP_IO_EXPANDER_HT8574_VER_PATCH    (0)

/**
 * @brief Create a HT8574 IO expander object
 *
 * @param[in]  i2c_bus    I2C bus handle. Obtained from `i2c_new_master_bus()`
 * @param[in]  i2c_dev_cfg   I2C device configuration.
 * @param[out] handle_ret Handle to created IO expander object
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
esp_err_t esp_io_expander_new_i2c_ht8574(
    i2c_master_bus_handle_t i2c_bus, const i2c_device_config_t *i2c_dev_cfg, esp_io_expander_handle_t *handle_ret
);

/**
 * @brief I2C address of the ht8574
 *
 * The 8-bit address format is as follows:
 *
 *                (Slave Address)
 *     ┌─────────────────┷─────────────────┐
 *  ┌─────┐─────┐─────┐─────┐─────┐─────┐─────┐─────┐
 *  |  0  |  1  |  1  |  1  | A2  | A1  | A0  | R/W |
 *  └─────┘─────┘─────┘─────┘─────┘─────┘─────┘─────┘
 *     └────────┯────────┘     └─────┯──────┘
 *           (Fixed)        (Hardware Selectable)
 *
 * And the 7-bit slave address is the most important data for users.
 * For example, if a chip's A0,A1,A2 are connected to GND, it's 7-bit slave address is 0111000b(0x38).
 * Then users can use `ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_000` to init it.
 */
#define ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_000    (0x38)
#define ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_001    (0x29)
#define ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_010    (0x2A)
#define ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_011    (0x2B)
#define ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_100    (0x2C)

/**
 * @brief Macro to create a I2C device configuration for HT8574
 *
 * @param[in] address I2C device address
 * @param[in] freq_hz I2C bus frequency
 * @return I2C device configuration
 */
#define ESP_IO_EXPANDER_I2C_HT8574_DEVICE_CFG(address, freq_hz) \
    { \
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, \
        .device_address = address, \
        .scl_speed_hz = freq_hz, \
    }

#ifdef __cplusplus
}
#endif
