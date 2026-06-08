/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_expander_base.hpp"

namespace esp_expander {

/**
 * @brief The Waveshare ESP32-S3-Touch-LCD-4 CH32V003 IO expander device class
 *
 * @note  This class is a derived class of `esp_expander::Base`, user can use it directly.
 * @note  The CH32V003 is a microcontroller acting as an I2C IO expander. The pin map below is
 *        specific to the firmware shipped on the Waveshare ESP32-S3-Touch-LCD-4 (4" square) board.
 * @note  Pin map (register bit -> function):
 *        | Pin Number | 1      | 2      | 3       | 4    | 5      | 6      | 7       |
 *        | ---------- | ------ | ------ | ------- | ---- | ------ | ------ | ------- |
 *        | Function   | TP_RST | TP_INT | LCD_RST | SDCD | SYS_EN | BEE_EN | RTC_INT |
 */
class WAVESHARE_LCD_4_CH32V003: public Base {
public:
    /**
     * @brief Pin (register bit) assignment for the LCD4 board firmware
     */
    enum Pin {
        PIN_TP_RST  = 1,    // EXIO1: touch panel reset
        PIN_TP_INT  = 2,    // EXIO2: touch panel interrupt / GT911 address strap
        PIN_LCD_RST = 3,    // EXIO3: LCD reset
        PIN_SD_CD   = 4,    // EXIO4: SD card detect
        PIN_SYS_EN  = 5,    // EXIO5: system / peripheral power enable
        PIN_BEE_EN  = 6,    // EXIO6: beeper enable
        PIN_RTC_INT = 7,    // EXIO7: RTC interrupt
    };

    /**
     * @brief Construct a device. With this function, call `init()` will initialize I2C by using the host
     *        configuration.
     *
     * @param[in] scl_io  I2C SCL pin number
     * @param[in] sda_io  I2C SDA pin number
     * @param[in] address I2C device 7-bit address. Should be `ESP_IO_EXPANDER_I2C_WAVESHARE_LCD_4_CH32V003_ADDRESS`.
     */
    WAVESHARE_LCD_4_CH32V003(int scl_io, int sda_io, uint8_t address): Base(scl_io, sda_io, address) {}

    /**
     * @brief Construct a device. With this function, call `init()` will not initialize I2C, and users should
     *        initialize it manually.
     *
     * @param[in] host_id I2C host ID.
     * @param[in] address I2C device 7-bit address. Should be `ESP_IO_EXPANDER_I2C_WAVESHARE_LCD_4_CH32V003_ADDRESS`.
     */
    WAVESHARE_LCD_4_CH32V003(int host_id, uint8_t address): Base(host_id, address) {}

    /**
     * @brief Construct a device.
     *
     * @param[in] config Configuration for the object
     */
    WAVESHARE_LCD_4_CH32V003(const Config &config): Base(config) {}

    /**
     * @brief Destruct object. This function will call `del()` to delete the object.
     */
    ~WAVESHARE_LCD_4_CH32V003() override;

    /**
     * @brief Begin object
     *
     * @note  This function typically calls `esp_io_expander_new_i2c_*()` to create the IO expander handle.
     *
     * @return true if success, otherwise false
     */
    bool begin(void) override;

    /**
     * @brief Set the backlight brightness
     *
     * @note  Backlight is driven by the CH32V003's dedicated PWM register, which is not part of the
     *        generic GPIO model.
     *
     * @param[in] percent Brightness percentage, 0 (off) to 100 (max)
     *
     * @return true if success, otherwise false
     */
    bool setBacklight(uint8_t percent);

    /**
     * @brief Turn the beeper on or off
     *
     * @note  The beeper is on EXIO6 (BEE_EN). The pin should be configured as output before use.
     *
     * @param[in] on true to enable the beeper, false to disable
     *
     * @return true if success, otherwise false
     */
    bool beep(bool on);
};

} // namespace esp_expander
