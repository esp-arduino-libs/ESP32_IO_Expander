/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_expander_base.hpp"

namespace esp_expander {

/**
 * @brief The TCA95XX_8BIT IO expander device class
 *
 * @note  This class is a derived class of `esp_expander::Base`, user can use it directly
 */
class TCA95XX_8BIT: public Base {
public:
    // /**
    //  * @brief Construct a TCA95XX_8BIT device. With this function, call `init()` will initialize I2C by using the host
    //  *        configuration.
    //  *
    //  * @param[in] scl_io  I2C SCL pin number
    //  * @param[in] sda_io  I2C SDA pin number
    //  * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_XXX` or
    //  *                     `ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_XXX`.
    //  */
    // TCA95XX_8BIT(int scl_io, int sda_io, uint8_t address)
    //     : Base(scl_io, sda_io, address)
    // {}

    // /**
    //  * @brief Construct a TCA95XX_8BIT device. With this function, call `init()` will not initialize I2C, and users should
    //  *        initialize it manually.
    //  *
    //  * @param[in] host_handle I2C host handle, which is created by `i2c_new_master_bus()`.
    //  * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_XXX` or
    //  *                     `ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_XXX`.
    //  */
    // TCA95XX_8BIT(HostHandle host_handle, uint8_t address)
    //     : Base(host_handle, address)
    // {}

    // /**
    //  * @brief Construct a TCA95XX_8BIT device.
    //  *
    //  * @param[in] config Configuration for the object
    //  */
    // TCA95XX_8BIT(const Config &config)
    //     : Base(config)
    // {}

    using Base::Base;

    /**
     * @brief Desutruct object. This function will call `del()` to delete the object.
     */
    ~TCA95XX_8BIT() override;

    /**
     * @brief Begin object
     *
     * @note  This function typically calls `esp_io_expander_new_i2c_tca9554()` to create the IO expander handle.
     * @note  This function sets all pins to input mode by default.
     *
     * @return true if success, otherwise false
     */
    bool begin() override;
};

} // namespace esp_expander
