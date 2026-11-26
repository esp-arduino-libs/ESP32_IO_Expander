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
    // /**
    //  * @brief Construct a HT8574 device. With this function, call `init()` will initialize I2C by using the host
    //  *        configuration.
    //  *
    //  * @param[in] scl_io  I2C SCL pin number
    //  * @param[in] sda_io  I2C SDA pin number
    //  * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_XXX`.
    //  */
    // HT8574(int scl_io, int sda_io, uint8_t address)
    //     : Base(scl_io, sda_io, address)
    // {}

    // /**
    //  * @brief Construct a HT8574 device. With this function, call `init()` will not initialize I2C, and users should
    //  *        initialize it manually.
    //  *
    //  * @param[in] host_handle I2C host handle, which is created by `i2c_new_master_bus()`.
    //  * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_HT8574_ADDRESS_XXX`.
    //  */
    // HT8574(HostHandle host_handle, uint8_t address)
    //     : Base(host_handle, address)
    // {}

    // /**
    //  * @brief Construct a HT8574 device.
    //  *
    //  * @param[in] config Configuration for the object
    //  */
    // HT8574(const Config &config)
    //     : Base(config)
    // {}

    using Base::Base;

    /**
     * @brief Desutruct object. This function will call `del()` to delete the object.
     */
    ~HT8574() override;

    /**
     * @brief Begin object
     *
     * @note  This function typically calls `esp_io_expander_new_i2c_ht8574()` to create the IO expander handle.
     *
     * @return true if success, otherwise false
     */
    bool begin() override;
};

} // namespace esp_expander
