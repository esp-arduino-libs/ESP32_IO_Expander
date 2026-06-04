/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "driver/i2c.h"
#include "esp_bit_defs.h"
#include "esp_check.h"
#include "esp_log.h"

#include "esp_io_expander.h"
#include "esp_io_expander_waveshare_lcd_4_ch32v003.h"

#include "esp_expander_utils.h"

/* Timeout of each I2C communication.
 * The CH32V003 briefly stretches/holds the bus when its beeper bit is toggled, so a short timeout
 * makes the transaction fail and drives the (legacy) I2C driver into i2c_hw_fsm_reset(), which can
 * trip the interrupt watchdog. 100 ms matches the Arduino `Wire` default that previously tolerated it. */
#define I2C_TIMEOUT_MS          (100)

#define IO_COUNT                (8)

/* Register address */
#define INPUT_REG_ADDR          (0x00)
#define OUTPUT_REG_ADDR         (0x02)
#define DIRECTION_REG_ADDR      (0x03)
#define PWM_REG_ADDR            (0x05)

/* Default register value (all pins input, output latch low) */
#define DIR_REG_DEFAULT_VAL     (0x00)   // 0 = input on this firmware
#define OUT_REG_DEFAULT_VAL     (0x00)

/**
 * @brief Device Structure Type
 */
typedef struct {
    esp_io_expander_t base;
    i2c_port_t i2c_num;
    uint32_t i2c_address;
    struct {
        uint8_t direction;
        uint8_t output;
    } regs;
} esp_io_expander_ch32v003_t;

static const char *TAG = "ch32v003_lcd4";

static esp_err_t read_input_reg(esp_io_expander_handle_t handle, uint32_t *value);
static esp_err_t write_output_reg(esp_io_expander_handle_t handle, uint32_t value);
static esp_err_t read_output_reg(esp_io_expander_handle_t handle, uint32_t *value);
static esp_err_t write_direction_reg(esp_io_expander_handle_t handle, uint32_t value);
static esp_err_t read_direction_reg(esp_io_expander_handle_t handle, uint32_t *value);
static esp_err_t reset(esp_io_expander_t *handle);
static esp_err_t del(esp_io_expander_t *handle);

esp_err_t esp_io_expander_new_i2c_waveshare_lcd_4_ch32v003(i2c_port_t i2c_num, uint32_t i2c_address,
        esp_io_expander_handle_t *handle)
{
    ESP_LOGI(TAG, "version: %d.%d.%d", ESP_IO_EXPANDER_WAVESHARE_LCD_4_CH32V003_VER_MAJOR,
             ESP_IO_EXPANDER_WAVESHARE_LCD_4_CH32V003_VER_MINOR, ESP_IO_EXPANDER_WAVESHARE_LCD_4_CH32V003_VER_PATCH);
    ESP_RETURN_ON_FALSE(i2c_num < I2C_NUM_MAX, ESP_ERR_INVALID_ARG, TAG, "Invalid i2c num");
    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "Invalid handle");

    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)calloc(1, sizeof(esp_io_expander_ch32v003_t));
    ESP_RETURN_ON_FALSE(ch32v003, ESP_ERR_NO_MEM, TAG, "Malloc failed");

    ch32v003->base.config.io_count = IO_COUNT;
    ch32v003->base.config.flags.dir_out_bit_zero = 0;   // 1 = output on this firmware
    ch32v003->i2c_num = i2c_num;
    ch32v003->i2c_address = i2c_address;
    ch32v003->base.read_input_reg = read_input_reg;
    ch32v003->base.write_output_reg = write_output_reg;
    ch32v003->base.read_output_reg = read_output_reg;
    ch32v003->base.write_direction_reg = write_direction_reg;
    ch32v003->base.read_direction_reg = read_direction_reg;
    ch32v003->base.del = del;
    ch32v003->base.reset = reset;

    esp_err_t ret = ESP_OK;
    /* Reset configuration and register status */
    ESP_GOTO_ON_ERROR(reset(&ch32v003->base), err, TAG, "Reset failed");

    *handle = &ch32v003->base;
    return ESP_OK;
err:
    free(ch32v003);
    return ret;
}

esp_err_t esp_io_expander_waveshare_lcd_4_ch32v003_set_pwm(esp_io_expander_handle_t handle, uint8_t duty)
{
    ESP_RETURN_ON_FALSE(handle, ESP_ERR_INVALID_ARG, TAG, "Invalid handle");
    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)__containerof(handle, esp_io_expander_ch32v003_t, base);

    uint8_t data[] = {PWM_REG_ADDR, duty};
    ESP_RETURN_ON_ERROR(
        i2c_master_write_to_device(ch32v003->i2c_num, ch32v003->i2c_address, data, sizeof(data), pdMS_TO_TICKS(I2C_TIMEOUT_MS)),
        TAG, "Write PWM reg failed");
    return ESP_OK;
}

static esp_err_t read_input_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)__containerof(handle, esp_io_expander_ch32v003_t, base);

    uint8_t temp = 0;
    // *INDENT-OFF*
    ESP_RETURN_ON_ERROR(
        i2c_master_write_read_device(ch32v003->i2c_num, ch32v003->i2c_address, (uint8_t[]){INPUT_REG_ADDR}, 1, &temp, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS)),
        TAG, "Read input reg failed");
    // *INDENT-ON*
    *value = temp;
    return ESP_OK;
}

static esp_err_t write_output_reg(esp_io_expander_handle_t handle, uint32_t value)
{
    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)__containerof(handle, esp_io_expander_ch32v003_t, base);
    value &= 0xff;

    uint8_t data[] = {OUTPUT_REG_ADDR, value};
    ESP_RETURN_ON_ERROR(
        i2c_master_write_to_device(ch32v003->i2c_num, ch32v003->i2c_address, data, sizeof(data), pdMS_TO_TICKS(I2C_TIMEOUT_MS)),
        TAG, "Write output reg failed");
    ch32v003->regs.output = value;
    return ESP_OK;
}

static esp_err_t read_output_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)__containerof(handle, esp_io_expander_ch32v003_t, base);

    *value = ch32v003->regs.output;
    return ESP_OK;
}

static esp_err_t write_direction_reg(esp_io_expander_handle_t handle, uint32_t value)
{
    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)__containerof(handle, esp_io_expander_ch32v003_t, base);
    value &= 0xff;

    uint8_t data[] = {DIRECTION_REG_ADDR, value};
    ESP_RETURN_ON_ERROR(
        i2c_master_write_to_device(ch32v003->i2c_num, ch32v003->i2c_address, data, sizeof(data), pdMS_TO_TICKS(I2C_TIMEOUT_MS)),
        TAG, "Write direction reg failed");
    ch32v003->regs.direction = value;
    return ESP_OK;
}

static esp_err_t read_direction_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)__containerof(handle, esp_io_expander_ch32v003_t, base);

    *value = ch32v003->regs.direction;
    return ESP_OK;
}

static esp_err_t reset(esp_io_expander_t *handle)
{
    ESP_RETURN_ON_ERROR(write_direction_reg(handle, DIR_REG_DEFAULT_VAL), TAG, "Write dir reg failed");
    ESP_RETURN_ON_ERROR(write_output_reg(handle, OUT_REG_DEFAULT_VAL), TAG, "Write output reg failed");
    return ESP_OK;
}

static esp_err_t del(esp_io_expander_t *handle)
{
    esp_io_expander_ch32v003_t *ch32v003 = (esp_io_expander_ch32v003_t *)__containerof(handle, esp_io_expander_ch32v003_t, base);

    free(ch32v003);
    return ESP_OK;
}
