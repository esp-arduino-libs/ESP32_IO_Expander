/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "private/esp_expander_utils.h"
#include "port/esp_io_expander_ht8574.h"
#include "esp_expander_ht8574.hpp"

namespace esp_expander {

HT8574::~HT8574()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_EXIT(del(), "Delete failed");
}

bool HT8574::begin()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(!isOverState(State::BEGIN), false, "Already begun");

    // Initialize the device if not initialized
    if (!isOverState(State::INIT)) {
        ESP_UTILS_CHECK_FALSE_RETURN(init(), false, "Init failed");
    }

    auto device_config = getDeviceFullConfig();
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_new_i2c_ht8574(getHostHandle(), device_config, &device_handle), false, "Create HT8574 failed"
    );
    ESP_UTILS_LOGD("Create HT8574 @%p", device_handle);

    setState(State::BEGIN);

    return true;
}

} // namespace esp_expander
