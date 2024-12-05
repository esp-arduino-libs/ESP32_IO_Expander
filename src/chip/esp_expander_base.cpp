/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "driver/i2c.h"
#include "esp_expander_utils.h"
#include "esp_expander_base.hpp"

// Check whether it is a valid pin number
#define IS_VALID_PIN(pin_num)   (pin_num < IO_COUNT_MAX)

namespace esp_expander {

bool Base::init(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(!checkIsInit(), false, "Already initialized");

    if (!checkIsSkipInitHost()) {
        ESP_UTILS_CHECK_ERROR_RETURN(i2c_param_config(getHostID(), &_host_config), false, "I2C param config failed");
        ESP_UTILS_CHECK_ERROR_RETURN(
            i2c_driver_install(getHostID(), _host_config.mode, 0, 0, 0), false, "I2C driver install failed"
        );
        ESP_UTILS_LOGI("Init I2C host(%d)", _host_id);
    }
    _flags.is_init = true;

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool Base::reset(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_reset(device_handle), false, "Reset failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool Base::del(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    if (checkIsBegun()) {
        ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_del(device_handle), false, "Delete failed");
        device_handle = nullptr;
        ESP_UTILS_LOGD("Delete IO expander(@%p)", device_handle);
    }

    if (checkIsInit()) {
        if (!checkIsSkipInitHost()) {
            ESP_UTILS_CHECK_ERROR_RETURN(i2c_driver_delete(getHostID()), false, "I2C driver delete failed");
            ESP_UTILS_LOGI("Delete I2C host(%d)", _host_id);
        }
        _flags.is_init = false;
    }

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool Base::pinMode(uint8_t pin, uint8_t mode)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin(%d), mode(%d)", pin, mode);
    ESP_UTILS_CHECK_FALSE_RETURN(IS_VALID_PIN(pin), false, "Invalid pin");
    ESP_UTILS_CHECK_FALSE_RETURN((mode == INPUT) || (mode == OUTPUT), false, "Invalid mode");

    esp_io_expander_dir_t dir = (mode == INPUT) ? IO_EXPANDER_INPUT : IO_EXPANDER_OUTPUT;
    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_set_dir(device_handle, BIT64(pin), dir), false, "Set dir failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool Base::digitalWrite(uint8_t pin, uint8_t value)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin(%d), value(%d)", pin, value);
    ESP_UTILS_CHECK_FALSE_RETURN(IS_VALID_PIN(pin), false, "Invalid pin");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_set_level(device_handle, BIT64(pin), value), false, "Set level failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

int Base::digitalRead(uint8_t pin)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin(%d)", pin);
    ESP_UTILS_CHECK_FALSE_RETURN(IS_VALID_PIN(pin), -1, "Invalid pin");

    uint32_t level = 0;
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_get_level(device_handle, BIT64(pin), &level), -1, "Get level failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return (level & BIT64(pin)) ? HIGH : LOW;
}

bool Base::multiPinMode(uint32_t pin_mask, uint8_t mode)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin_mask(%0x), mode(%d)", pin_mask, mode);
    ESP_UTILS_CHECK_FALSE_RETURN((mode == INPUT) || (mode == OUTPUT), false, "Invalid mode");

    esp_io_expander_dir_t dir = (mode == INPUT) ? IO_EXPANDER_INPUT : IO_EXPANDER_OUTPUT;
    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_set_dir(device_handle, pin_mask, dir), false, "Set dir failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool Base::multiDigitalWrite(uint32_t pin_mask, uint8_t value)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin_mask(%0x), value(%d)", pin_mask, value);

    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_set_level(device_handle, pin_mask, value), false, "Set level failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

int64_t Base::multiDigitalRead(uint32_t pin_mask)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin_mask(%0x)", pin_mask);

    uint32_t level = 0;
    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_get_level(device_handle, pin_mask, &level), false, "Get level failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return level;
}

bool Base::printStatus(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_print_state(device_handle), false, "Print state failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

} // namespace esp_expander
