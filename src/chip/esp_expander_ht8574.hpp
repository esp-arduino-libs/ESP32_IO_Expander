/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_expander_base.hpp"

namespace esp_expander {

/**
 * @brief The HT8574 IO expander device class
 *
 * @note  This class is a derived class of `esp_expander::Base`, user can use it directly
 */
class HT8574: public Base {
public:
    /**
     * @brief Construct a HT8574 device. With this function, call `init()` will initialize I2C by using the host
     *        configuration.
     *
     * @param[in] scl_io  I2C SCL pin number
     * @param[in] sda_io  I2C SDA pin number
     * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_<chip name>_ADDRESS`.
     */
    HT8574(int scl_io, int sda_io, uint8_t address): Base(scl_io, sda_io, address) {}

    /**
     * @brief Construct a HT8574 device. With this function, call `init()` will not initialize I2C, and users should
     *        initialize it manually.
     *
     * @param[in] host_id I2C host ID.
     * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_<chip name>_ADDRESS`.
     */
    HT8574(int host_id, uint8_t address): Base(host_id, address) {}

    /**
     * @brief Construct a HT8574 device.
     *
     * @param[in] config Configuration for the object
     */
    HT8574(const Config &config): Base(config) {}

    /**
     * @deprecated Deprecated and will be removed in the next major version. Please use other constructors instead.
     */
    [[deprecated("Deprecated and will be removed in the next major version. Please use other constructors instead.")]]
    HT8574(i2c_port_t id, uint8_t address, int scl_io, int sda_io): Base(id, address, scl_io, sda_io) {}

    /**
     * @brief Desutruct object. This function will call `del()` to delete the object.
     */
    ~HT8574() override;

    /**
     * @brief Begin object
     *
     * @note  This function typically calls `esp_io_expander_new_i2c_*()` to create the IO expander handle.
     * @note  The driver initialization by default sets CH422G's IO0-7 to output high-level mode.
     *
     * @return true if success, otherwise false
     */
    bool begin(void) override;
};

} // namespace esp_expander

/**
 * @deprecated Deprecated and will be removed in the next major version. Please use `esp_expander::HT8574` instead.
 */
typedef esp_expander::HT8574 ESP_IOExpander_HT8574 __attribute__((deprecated("Deprecated and will be removed in the next major version. Please use `esp_expander::HT8574` instead.")));
