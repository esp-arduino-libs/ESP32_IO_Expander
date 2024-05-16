/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"

#include "../ESP_IOExpander.h"

class ESP_IOExpander_CH422G: public ESP_IOExpander {
public:
    /**
     * @brief Constructor to create ESP_IOExpander object
     *
     * @note  After using this function, call `init()` will initialize I2C bus.
     *
     * @param id I2C port number
     * @param address I2C device address. Should be like `ESP_IO_EXPANDER_I2C_*`.
     *                Can be found in the header file of each IO expander.h.
     * @param config Pointer to I2C bus configuration
     */
    ESP_IOExpander_CH422G(i2c_port_t id, uint8_t address, const i2c_config_t *config): ESP_IOExpander(id, address, config) { };

    /**
     * @brief Constructor to create ESP_IOExpander object
     *
     * @note  After using this function, call `init()` will initialize I2C bus.
     *
     * @param id I2C port number
     * @param address I2C device address. Should be like `ESP_IO_EXPANDER_I2C_*`.
     *                Can be found in the header file of each IO expander.h.
     * @param scl SCL pin number
     * @param sda SDA pin number
     */
    ESP_IOExpander_CH422G(i2c_port_t id, uint8_t address, int scl, int sda): ESP_IOExpander(id, address, scl, sda) { };

    /**
     * @brief Constructor to create ESP_IOExpander object
     *
     * @note  If use this function, should initialize I2C bus before call `init()`.
     *
     * @param id I2C port number
     * @param address I2C device address. Should be like `ESP_IO_EXPANDER_I2C_*`.
     *                Can be found in the header file of each IO expander.h.
     */
    ESP_IOExpander_CH422G(i2c_port_t id, uint8_t address): ESP_IOExpander(id, address) { };

    /**
     * @brief Destructor
     *
     * @note  This function will delete I2C driver if it is initialized by ESP_IOExpander and delete ESP_IOExpander object.
     */
    ~ESP_IOExpander_CH422G() override;

    /**
     * @brief Begin IO expander
     *
     */
    void begin(void) override;
};

/**
 * @brief I2C address of the ch422g
 *
 * And the 7-bit slave address is the most important data for users.
 * For example, if a chip's A0,A1,A2 are connected to GND, it's 7-bit slave address is 1001000b(0x48).
 * Then users can use `ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000` to init it.
 */
#define ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000    (0x24)
