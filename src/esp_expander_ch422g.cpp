/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "private/esp_expander_utils.h"
#include "port/esp_io_expander_ch422g.h"
#include "esp_expander_ch422g.hpp"

namespace esp_expander {

CH422G::~CH422G()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_EXIT(del(), "Delete failed");
}

bool CH422G::begin()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(!isOverState(State::BEGIN), false, "Already begun");

    // Initialize the device if not initialized
    if (!isOverState(State::INIT)) {
        ESP_UTILS_CHECK_FALSE_RETURN(init(), false, "Init failed");
    }

    auto device_config = getDeviceFullConfig();
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_new_i2c_ch422g(getHostHandle(), device_config, &device_handle), false, "Create CH422G failed"
    );
    ESP_UTILS_LOGD("Create CH422G @%p", device_handle);

    setState(State::BEGIN);

    return true;
}

bool CH422G::enableOC_OpenDrain()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_oc_open_drain(device_handle), false, "Set OC open-drain failed"
    );

    return true;
}

bool CH422G::enableOC_PushPull()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_oc_push_pull(device_handle), false, "Set OC push-pull failed"
    );

    return true;
}

bool CH422G::enableAllIO_Input()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_all_input(device_handle), false, "Set all input failed"
    );

    return true;
}

bool CH422G::enableAllIO_Output()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_all_output(device_handle), false, "Set all output failed"
    );

    return true;
}

bool CH422G::enterSleep()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_enter_sleep(device_handle), false, "Enter sleep failed"
    );

    return true;
}

bool CH422G::exitSleep()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_exit_sleep(device_handle), false, "Exit sleep failed"
    );

    return true;
}

} // namespace esp_expander
