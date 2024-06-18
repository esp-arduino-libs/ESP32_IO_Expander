/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include "unity.h"
#include "unity_test_runner.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "ESP_IOExpander_Library.h"

static const char *TAG = "test_esp_io_expander";

/**
 * Create an ESP_IOExpander object, Currently supports:
 *      - TCA95xx_8bit
 *      - TCA95xx_16bit
 *      - HT8574
 *      - CH422G
 */
#define TEST_CHIP_NAME      TCA95xx_8bit
#define TEST_CHIP_ADDRESS   ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000
#define TEST_I2C_NUM        (0)
#define TEST_I2C_SDA_PIN    (8)
#define TEST_I2C_SCL_PIN    (18)

#define _TEST_CHIP_CLASS(name, ...)   ESP_IOExpander_##name(__VA_ARGS__)
#define TEST_CHIP_CLASS(name, ...)    _TEST_CHIP_CLASS(name, ##__VA_ARGS__)

#define TEST_FUNCTIONS_TIME_S   (5)

void test_i2c_init(void)
{
    const i2c_config_t i2c_config = EXPANDER_I2C_CONFIG_DEFAULT(TEST_I2C_SCL_PIN, TEST_I2C_SDA_PIN);
    TEST_ESP_OK(i2c_param_config((i2c_port_t)TEST_I2C_NUM, &i2c_config));
    TEST_ESP_OK(i2c_driver_install((i2c_port_t)TEST_I2C_NUM, i2c_config.mode, 0, 0, 0));
}

void test_i2c_deinit(void)
{
    TEST_ESP_OK(i2c_driver_delete((i2c_port_t)TEST_I2C_NUM));
}

void test_init_expander(ESP_IOExpander *expander, bool enable_external_i2c)
{
    if (enable_external_i2c) {
        ESP_LOGI(TAG, "Initialize I2C bus");
        test_i2c_init();
    }

    ESP_LOGI(TAG, "Test expander with external I2C");
    expander->init();
    expander->begin();
    expander->reset();
    expander->del();

    ESP_LOGI(TAG, "Delete the expander object");
    delete expander;

    if (enable_external_i2c) {
        ESP_LOGI(TAG, "Deinitialize I2C bus");
        test_i2c_deinit();
    }
}

TEST_CASE("test IO expander initialization", "[initialization]")
{
    ESP_IOExpander *expander = NULL;

    ESP_LOGI(TAG, "Test initialization with external I2C");
    expander = new TEST_CHIP_CLASS(TEST_CHIP_NAME, (i2c_port_t)TEST_I2C_NUM, TEST_CHIP_ADDRESS);
    test_init_expander(expander, true);

    ESP_LOGI(TAG, "Test initialization with internal I2C");
    expander = new TEST_CHIP_CLASS(TEST_CHIP_NAME, (i2c_port_t)TEST_I2C_NUM, TEST_CHIP_ADDRESS, TEST_I2C_SCL_PIN, TEST_I2C_SDA_PIN);
    test_init_expander(expander, false);
}

TEST_CASE("test IO expander operations", "[operations]")
{
    ESP_LOGI(TAG, "Create and initialize expander");
    ESP_IOExpander *expander = new TEST_CHIP_CLASS(TEST_CHIP_NAME, (i2c_port_t)TEST_I2C_NUM, TEST_CHIP_ADDRESS, TEST_I2C_SCL_PIN, TEST_I2C_SDA_PIN);
    expander->init();
    expander->begin();

    ESP_LOGI(TAG, "Original status:");
    expander->printStatus();

    expander->pinMode(0, OUTPUT);
    expander->pinMode(1, OUTPUT);
    expander->multiPinMode(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, OUTPUT);

    ESP_LOGI(TAG, "Set pint 0-3 to output mode:");
    expander->printStatus();

    expander->digitalWrite(0, LOW);
    expander->digitalWrite(1, LOW);
    expander->multiDigitalWrite(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, LOW);

    ESP_LOGI(TAG, "Set pint 0-3 to low level:");
    expander->printStatus();

    expander->pinMode(0, INPUT);
    expander->pinMode(1, INPUT);
    expander->multiPinMode(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, INPUT);

    ESP_LOGI(TAG, "Set pint 0-3 to input mode:");
    expander->printStatus();

    int level[4] = {0, 0, 0, 0};
    uint32_t level_temp;

    for (int i = 0; i < TEST_FUNCTIONS_TIME_S; i++) {
        // Read pin 0-3 level
        level[0] = expander->digitalRead(0);
        level[1] = expander->digitalRead(1);
        level_temp = expander->multiDigitalRead(IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3);
        level[2] = level_temp & IO_EXPANDER_PIN_NUM_2 ? HIGH : LOW;
        level[3] = level_temp & IO_EXPANDER_PIN_NUM_3 ? HIGH : LOW;
        ESP_LOGI(TAG, "Pin 0-3 level: %d %d %d %d", level[0], level[1], level[2], level[3]);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    delete expander;
}

// Some resources are lazy allocated in the LCD driver, the threadhold is left for that case
#define TEST_MEMORY_LEAK_THRESHOLD (-300)

static size_t before_free_8bit;
static size_t before_free_32bit;

static void check_leak(size_t before_free, size_t after_free, const char *type)
{
    ssize_t delta = after_free - before_free;
    printf("MALLOC_CAP_%s: Before %u bytes free, After %u bytes free (delta %d)\n", type, before_free, after_free, delta);
    TEST_ASSERT_MESSAGE(delta >= TEST_MEMORY_LEAK_THRESHOLD, "memory leak");
}

void setUp(void)
{
    before_free_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    before_free_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
}

void tearDown(void)
{
    size_t after_free_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t after_free_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    check_leak(before_free_8bit, after_free_8bit, "8BIT");
    check_leak(before_free_32bit, after_free_32bit, "32BIT");
}

extern "C" void app_main(void)
{
    //   _____  ___     __                            _
    //   \_   \/___\   /__\_  ___ __   __ _ _ __   __| | ___ _ __
    //    / /\//  //  /_\ \ \/ / '_ \ / _` | '_ \ / _` |/ _ \ '__|
    // /\/ /_/ \_//  //__  >  <| |_) | (_| | | | | (_| |  __/ |
    // \____/\___/   \__/ /_/\_\ .__/ \__,_|_| |_|\__,_|\___|_|
    //                         |_|
    printf("  _____  ___     __                            _\r\n");
    printf("  \\_   \\/___\\   /__\\_  ___ __   __ _ _ __   __| | ___ _ __\r\n");
    printf("   / /\\//  //  /_\\ \\ \\/ / '_ \\ / _` | '_ \\ / _` |/ _ \\ '__|\r\n");
    printf("/\\/ /_/ \\_//  //__  >  <| |_) | (_| | | | | (_| |  __/ |\r\n");
    printf("\\____/\\___/   \\__/ /_/\\_\\ .__/ \\__,_|_| |_|\\__,_|\\___|_|\r\n");
    printf("                        |_|\r\n");
    unity_run_menu();
}
