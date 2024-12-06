/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_expander_utils.h"
#include "port/esp_io_expander_ch422g.h"
#include "esp_expander_ch422g.hpp"

namespace esp_expander {

CH422G::~CH422G()
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_EXIT(del(), "Delete failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
}

bool CH422G::begin(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsInit(), false, "Not initialized");
    ESP_UTILS_CHECK_FALSE_RETURN(!checkIsBegun(), false, "Already begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_new_i2c_ch422g(getHostID(), getDeviceAddress(), &device_handle), false,
        "Create CH422G IO expander failed"
    );
    ESP_UTILS_LOGD("Create CH422G IO expander(@%p)", device_handle);

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool CH422G::enableOC_OpenDrain(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_oc_open_drain(device_handle), false, "Set OC open-drain failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool CH422G::enableOC_PushPull(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_oc_push_pull(device_handle), false, "Set OC push-pull failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool CH422G::enableAllIO_Input(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_all_input(device_handle), false, "Set all input failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool CH422G::enableAllIO_Output(void)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkIsBegun(), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_ch422g_set_all_output(device_handle), false, "Set all output failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

} // namespace esp_expander
