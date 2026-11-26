/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "unity.h"
#include "unity_test_runner.h"
#include "esp_io_expander.hpp"

using namespace esp_expander;

static const char *TAG = "general_test";

/* The following default configurations are for the board 'Espressif: ESP32_S3_LCD_EV_BOARD_V1_5, TCA9554' */
#define TEST_HOST_ID            (I2C_NUM_0)
#define TEST_HOST_I2C_SCL_PIN   (48)
#define TEST_HOST_I2C_SDA_PIN   (47)
#define TEST_DEVICE_ADDRESS     (ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000)

#define HOST_CONFIG_DEFAULT() \
    { \
        .i2c_port = TEST_HOST_ID, \
        .sda_io_num = static_cast<gpio_num_t>(TEST_HOST_I2C_SDA_PIN), \
        .scl_io_num = static_cast<gpio_num_t>(TEST_HOST_I2C_SCL_PIN), \
        .clk_source = I2C_CLK_SRC_DEFAULT, \
    }

#define _CREATE_DEVICE(name, ...) \
    ({ \
        ESP_LOGI(TAG, "Create device: " #name); \
        std::shared_ptr<Base> device = std::make_shared<name>(__VA_ARGS__); \
        TEST_ASSERT_NOT_NULL_MESSAGE(device, "Create device failed"); \
        device; \
    })
#define CREATE_DEVICE(name, ...) _CREATE_DEVICE(name, ##__VA_ARGS__)

static i2c_master_bus_handle_t init_host(void)
{
    const i2c_master_bus_config_t i2c_config = HOST_CONFIG_DEFAULT();
    i2c_master_bus_handle_t i2c_handle = nullptr;
    esp_err_t ret = i2c_new_master_bus(&i2c_config, &i2c_handle);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "I2C install returned error");
    return i2c_handle;
}

static void deinit_host(i2c_master_bus_handle_t i2c_handle)
{
    esp_err_t ret = i2c_del_master_bus(i2c_handle);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "I2C uninstall returned error");
}

static void test_device(std::shared_ptr<Base> device)
{
    TEST_ASSERT_MESSAGE(device->init(), "Device initialization failed");
    TEST_ASSERT_MESSAGE(device->begin(), "Device begin failed");
    TEST_ASSERT_MESSAGE(device->reset(), "Device reset failed");
    TEST_ASSERT_MESSAGE(device->del(), "Device del failed");
}

#define CREATE_TEST_CASE(device_name) \
    TEST_CASE("test " #device_name " general functions", "[io_expander][general][" #device_name "]") \
    { \
        std::shared_ptr<Base> expander = nullptr; \
        \
        ESP_LOGI(TAG, "Initialize I2C host"); \
        i2c_master_bus_handle_t i2c_handle = init_host(); \
        \
        ESP_LOGI(TAG, "Test constructor with (int host_id, uint8_t address) (external I2C)"); \
        expander = CREATE_DEVICE(device_name, TEST_HOST_ID, TEST_DEVICE_ADDRESS); \
        test_device(expander); \
        expander = nullptr; \
        \
        ESP_LOGI(TAG, "Test constructor with (const Config &config) (external I2C)"); \
        Base::Config external_i2c_config = { \
            .host_id = TEST_HOST_ID, \
            .device = Base::DevicePartialConfig{ \
                .address = TEST_DEVICE_ADDRESS, \
            }, \
        }; \
        expander = CREATE_DEVICE(device_name, external_i2c_config); \
        test_device(expander); \
        expander = nullptr; \
        \
        ESP_LOGI(TAG, "Deinitialize I2C host"); \
        deinit_host(i2c_handle); \
        \
        ESP_LOGI(TAG, "Test constructor with (int scl_io, int sda_io, uint8_t address) (internal I2C)"); \
        expander = CREATE_DEVICE(device_name, TEST_HOST_I2C_SCL_PIN, TEST_HOST_I2C_SDA_PIN, TEST_DEVICE_ADDRESS); \
        test_device(expander); \
        expander = nullptr; \
        \
        ESP_LOGI(TAG, "Test constructor with (const Config &config) (internal I2C)"); \
        Base::Config internal_i2c_config = { \
            .host_id = TEST_HOST_ID, \
            .host = Base::HostPartialConfig{ \
                .sda_io_num = static_cast<gpio_num_t>(TEST_HOST_I2C_SDA_PIN), \
                .scl_io_num = static_cast<gpio_num_t>(TEST_HOST_I2C_SCL_PIN), \
            }, \
            .device = Base::DevicePartialConfig{ \
                .address = TEST_DEVICE_ADDRESS, \
            }, \
        }; \
        expander = CREATE_DEVICE(device_name, internal_i2c_config); \
        test_device(expander); \
        expander = nullptr; \
        \
        expander = CREATE_DEVICE(device_name, TEST_HOST_I2C_SCL_PIN, TEST_HOST_I2C_SDA_PIN, TEST_DEVICE_ADDRESS); \
        TEST_ASSERT_MESSAGE(expander->init(), "Device initialization failed"); \
        TEST_ASSERT_MESSAGE(expander->begin(), "Device begin failed"); \
        \
        ESP_LOGI(TAG, "Test input/output functions"); \
        ESP_LOGI(TAG, "Original status:"); \
        TEST_ASSERT_MESSAGE(expander->printStatus(), "Print status failed"); \
        \
        TEST_ASSERT_MESSAGE(expander->pinMode(0, OUTPUT), "Set pin 0 to output mode failed"); \
        TEST_ASSERT_MESSAGE(expander->pinMode(1, OUTPUT), "Set pin 1 to output mode failed"); \
        TEST_ASSERT_MESSAGE( \
            expander->multiPinMode(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, OUTPUT), \
            "Set pin 2-3 to output mode failed" \
        ); \
        \
        ESP_LOGI(TAG, "Set pint 0-3 to output mode:"); \
        TEST_ASSERT_MESSAGE(expander->printStatus(), "Print status failed"); \
        \
        TEST_ASSERT_MESSAGE(expander->digitalWrite(0, LOW), "Set pin 0 to low level failed"); \
        TEST_ASSERT_MESSAGE(expander->digitalWrite(1, LOW), "Set pin 1 to low level failed"); \
        TEST_ASSERT_MESSAGE( \
            expander->multiDigitalWrite(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, LOW), \
            "Set pin 2-3 to low level failed" \
        ); \
        \
        ESP_LOGI(TAG, "Set pint 0-3 to low level:"); \
        TEST_ASSERT_MESSAGE(expander->printStatus(), "Print status failed"); \
        \
        TEST_ASSERT_MESSAGE(expander->pinMode(0, INPUT), "Set pin 0 to input mode failed"); \
        TEST_ASSERT_MESSAGE(expander->pinMode(1, INPUT), "Set pin 1 to input mode failed"); \
        TEST_ASSERT_MESSAGE( \
            expander->multiPinMode(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, INPUT), \
            "Set pin 2-3 to input mode failed" \
        ); \
        \
        ESP_LOGI(TAG, "Set pint 0-3 to input mode:"); \
        TEST_ASSERT_MESSAGE(expander->printStatus(), "Print status failed"); \
        \
        int level[4] = {0, 0, 0, 0}; \
        int64_t level_temp; \
        \
        level[0] = expander->digitalRead(0); \
        TEST_ASSERT_MESSAGE(level[0] >= 0, "Read pin 0 level failed"); \
        level[1] = expander->digitalRead(1); \
        TEST_ASSERT_MESSAGE(level[1] >= 0, "Read pin 0 level failed"); \
        level_temp = expander->multiDigitalRead(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3); \
        TEST_ASSERT_MESSAGE(level_temp >= 0, "Read pin 2-3 level failed"); \
        level[2] = level_temp & IO_EXPANDER_PIN_NUM_2 ? HIGH : LOW; \
        level[3] = level_temp & IO_EXPANDER_PIN_NUM_3 ? HIGH : LOW; \
        ESP_LOGI(TAG, "Pin 0-3 level: %d %d %d %d", level[0], level[1], level[2], level[3]); \
    }

/**
 * Here to create test cases for different devices
 */
CREATE_TEST_CASE(TCA95XX_8BIT)
CREATE_TEST_CASE(TCA95XX_16BIT)
CREATE_TEST_CASE(CH422G)
CREATE_TEST_CASE(HT8574)
