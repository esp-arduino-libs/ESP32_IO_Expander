/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "driver/i2c.h"
#include "port/esp_io_expander.h"

// Refer to `esp32-hal-gpio.h` in Arduino
#ifndef INPUT
#define INPUT             0x01
#endif
#ifndef OUTPUT
#define OUTPUT            0x03
#endif
#ifndef LOW
#define LOW               0x0
#endif
#ifndef HIGH
#define HIGH              0x1
#endif

namespace esp_expander {

/**
 * @brief The IO expander device class
 *
 * @note  This is a base class for all chips. Due to it is a virtual class, users cannot use it directly
 *
 */
class Base {
public:
    /* Default I2C host ID */
    constexpr static int HOST_ID_DEFAULT = static_cast<int>(I2C_NUM_0);

    /**
     * @brief Configuration for Base object
     *
     */
    struct Config {
        i2c_port_t getHostID(void) const
        {
            return static_cast<i2c_port_t>(host_id);
        }

        i2c_config_t getHostConfig(void) const
        {
            return {
                .mode = I2C_MODE_MASTER,
                .sda_io_num = host_sda_io_num,
                .scl_io_num = host_scl_io_num,
                .sda_pullup_en = host_sda_pullup_en,
                .scl_pullup_en = host_scl_pullup_en,
                .master = {
                    .clk_speed = host_clk_speed,
                },
                .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL,
            };
        }

        uint8_t getDeviceAddress(void) const
        {
            return device_address;
        }

        static Config create(int scl_io, int sda_io, uint8_t address)
        {
            return Config{
                .host_sda_io_num = sda_io,
                .host_scl_io_num = scl_io,
                .device_address = address,
                .skip_init_host = false,
            };
        }

        static Config create(int host_id, uint8_t address)
        {
            return Config{
                .host_id = host_id,
                .device_address = address,
                .skip_init_host = true,
            };
        }

        // Host
        int host_id = HOST_ID_DEFAULT;                  /*!< I2C host ID */
        int host_sda_io_num = -1;                       /*!< I2C SDA pin number */
        int host_scl_io_num = -1;                       /*!< I2C SCL pin number */
        bool host_sda_pullup_en = GPIO_PULLUP_ENABLE;   /*!< I2C SDA pullup enable */
        bool host_scl_pullup_en = GPIO_PULLUP_ENABLE;   /*!< I2C SCL pullup enable */
        uint32_t host_clk_speed = 400000;               /*!< I2C clock speed */
        // Device
        uint8_t device_address = 0;                     /*!< I2C device 7-bit address */
        // Extra
        bool skip_init_host = false;                    /*!< Skip I2C initialization when call `init()` */
    };

    /**
     * @brief Construct a base device. With this function, call `init()` will initialize I2C by using the host
     *        configuration.
     *
     * @param[in] scl_io  I2C SCL pin number
     * @param[in] sda_io  I2C SDA pin number
     * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_<chip name>_ADDRESS`.
     *
     */
    Base(int scl_io, int sda_io, uint8_t address)
    {
        auto config = Config::create(scl_io, sda_io, address);
        _host_id = config.getHostID();
        _host_config = config.getHostConfig();
        _device_address = config.getDeviceAddress();
        _flags.skip_init_host = config.skip_init_host;
    }

    /**
     * @brief Construct a base device. With this function, call `init()` will not initialize I2C, and users should
     *        initialize it manually.
     *
     * @param[in] host_id I2C host ID.
     * @param[in] address I2C device 7-bit address. Should be like `ESP_IO_EXPANDER_I2C_<chip name>_ADDRESS`.
     *
     */
    Base(int host_id, uint8_t address)
    {
        auto config = Config::create(host_id, address);
        _host_id = config.getHostID();
        _host_config = config.getHostConfig();
        _device_address = config.getDeviceAddress();
        _flags.skip_init_host = config.skip_init_host;
    }

    /**
     * @brief Construct a base device.
     *
     * @param[in] config Configuration for the object
     *
     */
    Base(const Config &config)
    {
        _host_id = config.getHostID();
        _host_config = config.getHostConfig();
        _device_address = config.getDeviceAddress();
        _flags.skip_init_host = config.skip_init_host;
    }

    /**
     * @deprecated Deprecated and will be removed in the next major version. Please use other constructors instead.
     *
     */
    Base(i2c_port_t id, uint8_t address, int scl_io, int sda_io):
        Base(scl_io, sda_io, address)
    {
        _host_id = id;
    }

    /**
     * @brief Virtual desutruct object.
     *
     * @note  Here make it virtual so that we can delete the derived object by using the base pointer.
     *
     */
    virtual ~Base() = default;

    /**
     * @brief Initialize object
     *
     * @note  This function will initialize I2C if needed.
     *
     * @return true if success, otherwise false
     *
     */
    bool init(void);

    /**
     * @brief Begin object
     *
     * @note  This function typically calls `esp_io_expander_new_i2c_*()` to create the IO expander handle.
     *
     */
    virtual bool begin(void) = 0;

    /**
     * @brief Reset object
     *
     * @return true if success, otherwise false
     *
     */
    bool reset(void);

    /**
     * @brief Delete object
     *
     */
    bool del(void);

    /**
     * @brief Set pin mode
     *
     * @note  This function is same as Arduino's `pinMode()`.
     *
     * @param[in] pin  Pin number (0-31)
     * @param[in] mode Pin mode (INPUT / OUTPUT)
     *
     * @return true if success, otherwise false
     *
     */
    bool pinMode(uint8_t pin, uint8_t mode);

    /**
     * @brief Set pin level
     *
     * @note  This function is same as Arduino's `digitalWrite()`.
     *
     * @param[in] pin   Pin number (0-31)
     * @param[in] value Pin level (HIGH / LOW)
     *
     * @return true if success, otherwise false
     *
     */
    bool digitalWrite(uint8_t pin, uint8_t value);

    /**
     * @brief Read pin level
     *
     * @note  This function is same as Arduino's `digitalRead()`.
     *
     * @param[in] pin Pin number (0-31)
     *
     * @return Pin level. HIGH or LOW if success, otherwise -1
     *
     */
    int digitalRead(uint8_t pin);

    /**
     * @brief Set multiple pin modes
     *
     * @param pin_mask Pin mask (Bitwise OR of `IO_EXPANDER_PIN_NUM_*`)
     * @param mode     Mode to set (INPUT / OUTPUT)
     *
     * @return true if success, otherwise false
     *
     */
    bool multiPinMode(uint32_t pin_mask, uint8_t mode);

    /**
     * @brief Set multiple pins level
     *
     * @param pin_mask Pin mask (Bitwise OR of `IO_EXPANDER_PIN_NUM_*`)
     * @param value Value to write (HIGH / LOW)
     *
     * @return true if success, otherwise false
     *
     */
    bool multiDigitalWrite(uint32_t pin_mask, uint8_t value);

    /**
     * @brief Read multiple pin levels
     *
     * @param pin_mask Pin mask (Bitwise OR of `IO_EXPANDER_PIN_NUM_*`)
     *
     * @return Pin levels, every bit represents a pin (HIGH / LOW)
     *
     */
    int64_t multiDigitalRead(uint32_t pin_mask);

    /**
     * @brief Print IO expander status, include pin index, direction, input level and output level
     *
     * @return Pin levels, every bit represents a pin (HIGH / LOW)
     *
     */
    bool printStatus(void);

    /**
     * @brief Get low-level handle. Users can use this handle to call low-level functions (esp_io_expander_*()).
     *
     * @return Handle if success, otherwise nullptr
     *
     */
    esp_io_expander_handle_t getDeviceHandle(void)
    {
        return device_handle;
    }

    /**
     * @deprecated Deprecated and will be removed in the next major version. Please use `getDeviceHandle()` instead.
     *
     */
    [[deprecated("Deprecated and will be removed in the next major version. Please use `getDeviceHandle()` instead.")]]
    esp_io_expander_handle_t getHandle(void)
    {
        return getDeviceHandle();
    }

protected:
    bool checkIsInit(void)
    {
        return _flags.is_init;
    }

    bool checkIsBegun(void)
    {
        return (device_handle != nullptr);
    }

    bool checkIsSkipInitHost(void)
    {
        return _flags.skip_init_host;
    }

    i2c_port_t getHostID(void)
    {
        return static_cast<i2c_port_t>(_host_id);
    }

    const i2c_config_t &getHostConfig(void)
    {
        return _host_config;
    }

    uint8_t getDeviceAddress(void)
    {
        return _device_address;
    }

    esp_io_expander_handle_t device_handle = nullptr;

private:
    struct {
        uint8_t is_init: 1;
        uint8_t skip_init_host: 1;
    } _flags = {};
    // Host
    int _host_id = HOST_ID_DEFAULT;
    i2c_config_t _host_config = {};
    // Device
    uint8_t _device_address = 0;
};

} // namespace esp_expander

/**
 * @deprecated Deprecated and will be removed in the next major version. Please use `esp_expander::Base` instead.
 *
 */
typedef esp_expander::Base ESP_IOExpander __attribute__((deprecated("Deprecated and will be removed in the next major version. Please use `esp_expander::Base` instead.")));
