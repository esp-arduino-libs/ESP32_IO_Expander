/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "inttypes.h"
#include "private/esp_expander_utils.h"
#include "esp_expander_base.hpp"

// Check whether it is a valid pin number
#define IS_VALID_PIN(pin_num)   (pin_num < IO_COUNT_MAX)

namespace esp_expander {

void Base::Config::convertPartialToFull()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    if (isHostConfigValid() && std::holds_alternative<HostPartialConfig>(host.value())) {
#if ESP_UTILS_CONF_LOG_LEVEL == ESP_UTILS_LOG_LEVEL_DEBUG
        dumpHost();
#endif // ESP_UTILS_LOG_LEVEL_DEBUG
        auto &config = std::get<HostPartialConfig>(host.value());
        host = HostFullConfig{
            .i2c_port = static_cast<i2c_port_t>(host_id),
            .sda_io_num = static_cast<gpio_num_t>(config.sda_io_num),
            .scl_io_num = static_cast<gpio_num_t>(config.scl_io_num),
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags = {
                .enable_internal_pullup = config.enable_internal_pullup,
            },
        };
    }

    if (std::holds_alternative<DevicePartialConfig>(device)) {
        auto &config = std::get<DevicePartialConfig>(device);
        device = DeviceFullConfig{
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = static_cast<uint16_t>(config.address),
            .scl_speed_hz = static_cast<uint32_t>(config.freq_hz),
        };
    }
}

void Base::Config::dumpHost() const
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    if (!isHostConfigValid()) {
        ESP_UTILS_LOGI("\n\t{Host config}[skipped]");
        return;
    }

    if (std::holds_alternative<HostFullConfig>(host.value())) {
        auto &config = std::get<HostFullConfig>(host.value());
        ESP_UTILS_LOGI(
            "\n\t{Host config}[full]\n"
            "\t\t-> [i2c_port]: %d\n"
            "\t\t-> [sda_io_num]: %d\n"
            "\t\t-> [scl_io_num]: %d\n"
            "\t\t-> [clk_source]: %d\n"
            "\t\t-> [glitch_ignore_cnt]: %d\n"
            "\t\t-> [intr_priority]: %d\n"
            "\t\t-> [trans_queue_depth]: %d\n"
            "\t\t-> [flags]:\n"
            "\t\t\t-> [enable_internal_pullup]: %d\n"
            "\t\t\t-> [allow_pd]: %d\n"
            , static_cast<int>(config.i2c_port)
            , static_cast<int>(config.sda_io_num)
            , static_cast<int>(config.scl_io_num)
            , static_cast<int>(config.clk_source)
            , static_cast<int>(config.glitch_ignore_cnt)
            , static_cast<int>(config.intr_priority)
            , static_cast<int>(config.trans_queue_depth)
            , static_cast<int>(config.flags.enable_internal_pullup)
            , static_cast<int>(config.flags.allow_pd)
        );
    } else {
        auto &config = std::get<HostPartialConfig>(host.value());
        ESP_UTILS_LOGI(
            "\n\t{Host config}[partial]\n"
            "\t\t-> [id]: %d\n"
            "\t\t-> [sda_io_num]: %d\n"
            "\t\t-> [scl_io_num]: %d\n"
            "\t\t-> [enable_internal_pullup]: %d\n"
            , static_cast<int>(host_id)
            , static_cast<int>(config.sda_io_num)
            , static_cast<int>(config.scl_io_num)
            , static_cast<int>(config.enable_internal_pullup)
        );
    }
}

void Base::Config::dumpDevice() const
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    if (std::holds_alternative<DeviceFullConfig>(device)) {
        auto &config = std::get<DeviceFullConfig>(device);
        ESP_UTILS_LOGI(
            "\n\t{Device config}[full]\n"
            "\t\t-> [dev_addr_length]: %d\n"
            "\t\t-> [device_address]: 0x%02X\n"
            "\t\t-> [scl_speed_hz]: %d\n"
            "\t\t-> [scl_wait_us]: %d\n"
            "\t\t-> [flags]:\n"
            "\t\t\t-> [disable_ack_check]: %d\n"
            , static_cast<int>(config.dev_addr_length)
            , static_cast<int>(config.device_address)
            , static_cast<int>(config.scl_speed_hz)
            , static_cast<int>(config.scl_wait_us)
            , static_cast<int>(config.flags.disable_ack_check)
        );
    } else {
        auto &config = std::get<DevicePartialConfig>(device);
        ESP_UTILS_LOGI(
            "\n\t{Device config}[partial]\n"
            "\t\t-> [address]: 0x%02X\n"
            "\t\t-> [freq_hz]: %d\n"
            , static_cast<int>(config.address)
            , static_cast<int>(config.freq_hz)
        );
    }
}

bool Base::configHostSkipInit(bool skip_init)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(!isOverState(State::INIT), false, "Should be called before `init()`");

    flags_.is_host_skip_init = skip_init;

    return true;
}

bool Base::init()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(!isOverState(State::INIT), false, "Already initialized");

    // Convert the partial configuration to full configuration
    config_.convertPartialToFull();
#if ESP_UTILS_CONF_LOG_LEVEL == ESP_UTILS_LOG_LEVEL_DEBUG
    config_.dumpHost();
    config_.dumpDevice();
#endif // ESP_UTILS_LOG_LEVEL_DEBUG

    // Initialize the I2C host if not initialized manually and host config is valid
    if (!flags_.is_host_skip_init && config_.isHostConfigValid()) {
        ESP_UTILS_LOGD("Try to init I2C host(%d)", static_cast<int>(config_.host_id));

        auto host_config = getHostFullConfig();
        ESP_UTILS_CHECK_NULL_RETURN(host_config, false, "Host config is not valid");

        ESP_UTILS_CHECK_ERROR_RETURN(
            i2c_new_master_bus(host_config, &host_handle_), false, "I2C new master bus failed"
        );
        ESP_UTILS_LOGD("Init I2C host(%d)(@%p)", static_cast<int>(config_.host_id), host_handle_);
    }

    setState(State::INIT);

    return true;
}

bool Base::reset()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_reset(device_handle), false, "Reset failed");

    return true;
}

bool Base::del()
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    if (device_handle != nullptr) {
        ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_del(device_handle), false, "Delete failed");
        ESP_UTILS_LOGD("Delete device @%p", device_handle);
        device_handle = nullptr;
    }

    if (host_handle_ != nullptr) {
        if (!flags_.is_host_skip_init && config_.isHostConfigValid()) {
            ESP_UTILS_CHECK_ERROR_RETURN(i2c_del_master_bus(host_handle_), false, "I2C driver delete failed");
        }
        ESP_UTILS_LOGD("Delete host @%p", host_handle_);
        host_handle_ = nullptr;
    }

    setState(State::DEINIT);

    return true;
}

bool Base::pinMode(uint8_t pin, uint8_t mode)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin(%d), mode(%d)", pin, mode);
    ESP_UTILS_CHECK_FALSE_RETURN(IS_VALID_PIN(pin), false, "Invalid pin");
    ESP_UTILS_CHECK_FALSE_RETURN((mode == INPUT) || (mode == OUTPUT), false, "Invalid mode");

    esp_io_expander_dir_t dir = (mode == INPUT) ? IO_EXPANDER_INPUT : IO_EXPANDER_OUTPUT;
    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_set_dir(device_handle, BIT64(pin), dir), false, "Set dir failed");

    return true;
}

bool Base::digitalWrite(uint8_t pin, uint8_t value)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin(%d), value(%d)", pin, value);
    ESP_UTILS_CHECK_FALSE_RETURN(IS_VALID_PIN(pin), false, "Invalid pin");

    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_set_level(device_handle, BIT64(pin), value), false, "Set level failed"
    );

    return true;
}

int Base::digitalRead(uint8_t pin)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin(%d)", pin);
    ESP_UTILS_CHECK_FALSE_RETURN(IS_VALID_PIN(pin), -1, "Invalid pin");

    uint32_t level = 0;
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_io_expander_get_level(device_handle, BIT64(pin), &level), -1, "Get level failed"
    );

    return (level & BIT64(pin)) ? HIGH : LOW;
}

bool Base::multiPinMode(uint32_t pin_mask, uint8_t mode)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin_mask(0x%" PRIx32 "), mode(%d)", pin_mask, mode);
    ESP_UTILS_CHECK_FALSE_RETURN((mode == INPUT) || (mode == OUTPUT), false, "Invalid mode");

    esp_io_expander_dir_t dir = (mode == INPUT) ? IO_EXPANDER_INPUT : IO_EXPANDER_OUTPUT;
    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_set_dir(device_handle, pin_mask, dir), false, "Set dir failed");

    return true;
}

bool Base::multiDigitalWrite(uint32_t pin_mask, uint8_t value)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin_mask(0x%" PRIx32 "), value(%d)", pin_mask, value);

    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_set_level(device_handle, pin_mask, value), false, "Set level failed");

    return true;
}

int64_t Base::multiDigitalRead(uint32_t pin_mask)
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_LOGD("Param: pin_mask(0x%" PRIx32 ")", pin_mask);

    uint32_t level = 0;
    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_get_level(device_handle, pin_mask, &level), false, "Get level failed");

    return level;
}

bool Base::printStatus() const
{
    ESP_UTILS_LOG_TRACE_GUARD_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(isOverState(State::BEGIN), false, "Not begun");

    ESP_UTILS_CHECK_ERROR_RETURN(esp_io_expander_print_state(device_handle), false, "Print state failed");

    return true;
}

Base::DeviceFullConfig *Base::getDeviceFullConfig()
{
    if (std::holds_alternative<DevicePartialConfig>(config_.device)) {
        config_.convertPartialToFull();
    }

    return &std::get<DeviceFullConfig>(config_.device);
}

Base::HostFullConfig *Base::getHostFullConfig()
{
    ESP_UTILS_CHECK_FALSE_RETURN(config_.isHostConfigValid(), nullptr, "Host config is not valid");

    if (std::holds_alternative<HostPartialConfig>(config_.host.value())) {
        config_.convertPartialToFull();
    }

    return &std::get<HostFullConfig>(config_.host.value());
}

Base::HostHandle Base::getHostHandle()
{
    if (host_handle_ != nullptr) {
        return host_handle_;
    }

    ESP_UTILS_LOGD("Try to get I2C host(%d) handle", static_cast<int>(config_.host_id));
    ESP_UTILS_CHECK_ERROR_RETURN(
        i2c_master_get_bus_handle(static_cast<i2c_port_t>(config_.host_id), &host_handle_),
        nullptr, "I2C master bus handle get failed"
    );

    return host_handle_;
}

} // namespace esp_expander
