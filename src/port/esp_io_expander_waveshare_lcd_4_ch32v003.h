/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP IO expander: Waveshare ESP32-S3-Touch-LCD-4 CH32V003
 *
 * @note  The CH32V003 is a microcontroller used as an I2C IO expander. The register map and pin
 *        assignment implemented here are specific to the firmware shipped on the Waveshare
 *        ESP32-S3-Touch-LCD-4 (4" square) board. Other boards may use a CH32V003 with different
 *        firmware, so this driver is intentionally board-scoped.
 *
 * @note  Register map (board firmware):
 *        | Address | Register   | Notes                                    |
 *        | ------- | ---------- | ---------------------------------------- |
 *        | 0x00    | INPUT      | Read-only pin input levels               |
 *        | 0x02    | OUTPUT     | Output latch (readable)                  |
 *        | 0x03    | DIRECTION  | 1 = output, 0 = input                    |
 *        | 0x05    | PWM        | Backlight PWM, 0 = max .. 247 = off       |
 *
 * @note  Pin map (register bit -> function):
 *        | Pin (bit) | 1      | 2      | 3       | 4    | 5      | 6      | 7       |
 *        | --------- | ------ | ------ | ------- | ---- | ------ | ------ | ------- |
 *        | Function  | TP_RST | TP_INT | LCD_RST | SDCD | SYS_EN | BEE_EN | RTC_INT |
 */

#pragma once

#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"

#include "esp_io_expander.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_IO_EXPANDER_WAVESHARE_LCD_4_CH32V003_VER_MAJOR    (1)
#define ESP_IO_EXPANDER_WAVESHARE_LCD_4_CH32V003_VER_MINOR    (0)
#define ESP_IO_EXPANDER_WAVESHARE_LCD_4_CH32V003_VER_PATCH    (0)

/**
 * @brief I2C address of the CH32V003 expander on the Waveshare ESP32-S3-Touch-LCD-4 board
 */
#define ESP_IO_EXPANDER_I2C_WAVESHARE_LCD_4_CH32V003_ADDRESS    (0x24)

/**
 * @brief Create a new Waveshare ESP32-S3-Touch-LCD-4 CH32V003 IO expander driver
 *
 * @note The I2C communication should be initialized before use this function
 *
 * @param i2c_num: I2C port num
 * @param i2c_address: I2C address of chip
 * @param handle: IO expander handle
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
esp_err_t esp_io_expander_new_i2c_waveshare_lcd_4_ch32v003(i2c_port_t i2c_num, uint32_t i2c_address,
        esp_io_expander_handle_t *handle);

/**
 * @brief Set the backlight PWM register
 *
 * @note The PWM register is dedicated to backlight control and is not part of the generic GPIO
 *       model. The duty is inverted by the firmware: 0 = maximum brightness, 247 = off.
 *
 * @param handle: IO expander handle
 * @param duty: Raw PWM register value (0 = max brightness .. 247 = off)
 *
 * @return
 *      - ESP_OK: Success, otherwise returns ESP_ERR_xxx
 */
esp_err_t esp_io_expander_waveshare_lcd_4_ch32v003_set_pwm(esp_io_expander_handle_t handle, uint8_t duty);

#ifdef __cplusplus
}
#endif
