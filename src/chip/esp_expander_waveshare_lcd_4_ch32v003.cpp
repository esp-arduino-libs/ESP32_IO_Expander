/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_expander_utils.h"
#include "port/esp_io_expander_waveshare_lcd_4_ch32v003.h"
#include "esp_expander_waveshare_lcd_4_ch32v003.hpp"

namespace esp_expander {

WAVESHARE_LCD_4_CH32V003::~WAVESHARE_LCD_4_CH32V003()
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_EXIT(del(), "Delete failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
}

bool WAVESHARE_LCD_4_CH32V003::begin(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(!isOverState(State::BEGIN), false, "Already begun");

    // Initialize the device if not initialized
    if (!isOverState(State::INIT)) {
        ESP_UTILS_CHECK_FALSE_RETURN(init(), false, "Init failed");
    }

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_new_i2c_waveshare_lcd_4_ch32v003(
            static_cast<i2c_port_t>(getConfig().host_id), getConfig().device.address, &device_handle
        ), false, "Create WAVESHARE_LCD_4_CH32V003 failed"
    );
    ESP_UTILS_LOGD("Create WAVESHARE_LCD_4_CH32V003 @%p", device_handle);

    setState(State::BEGIN);

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool WAVESHARE_LCD_4_CH32V003::setBacklight(uint8_t percent)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    if (percent > 100) {
        percent = 100;
    }
    // PWM register is inverted: 0 = max brightness, 247 = off
    uint8_t duty = (percent == 0) ? 247 : (uint8_t)(247 - ((uint32_t)percent * 247 / 100));

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_waveshare_lcd_4_ch32v003_set_pwm(device_handle, duty), false, "Set backlight failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool WAVESHARE_LCD_4_CH32V003::beep(bool on)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_FALSE_RETURN(digitalWrite(PIN_BEE_EN, on ? HIGH : LOW), false, "Set beep failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

} // namespace esp_expander
