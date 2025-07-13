/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_bit_defs.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_io_expander.h"
#include "esp_io_expander_ch422g.h"
#include "private/esp_expander_utils.h"

/* I2C communication related */
#define I2C_TIMEOUT_MS          (1000)

#define IO_COUNT                (12)

/* Register address */
#define CH422G_REG_ADDR_WR_SET  (0x48 >> 1)
#define CH422G_REG_ADDR_WR_OC   (0x46 >> 1)
#define CH422G_REG_ADDR_WR_IO   (0x70 >> 1)
#define CH422G_REG_ADDR_RD_IO   (0x4D >> 1)

/* Default register value when reset */
// *INDENT-OFF*
#define REG_WR_SET_DEFAULT_VAL  (0x01UL)    // Bit:        |  7  |  6  |  5  |  4  |    3    |    2    |    1     |    0    |
                                            //             | --- | --- | --- | --- | ------- | ------- | -------- | ------- |
                                            // Value:      |  /  |  /  |  /  |  /  | [SLEEP] | [OD_EN] | [A_SCAN] | [IO_OE] |
                                            //             | --- | --- | --- | --- | ------- | ------- | -------- | ------- |
                                            // Default:    |  0  |  0  |  0  | 0   |    0    |    0    |    0     |    1    |

// *INDENT-OFF*
#define REG_WR_OC_DEFAULT_VAL   (0x0FU)
#define REG_WR_IO_DEFAULT_VAL   (0xFFU)
#define REG_OUT_DEFAULT_VAL     ((REG_WR_OC_DEFAULT_VAL << 8) | REG_WR_IO_DEFAULT_VAL)
#define REG_DIR_DEFAULT_VAL     (0xFFFU)

#define REG_WR_SET_BIT_IO_OE    (1U << 0)
#define REG_WR_SET_BIT_OD_EN    (1U << 2)
#define REG_WR_SET_BIT_SLEEP    (1U << 3)

#define DIR_OUT_VALUE           (0xFFF)
#define DIR_IN_VALUE            (0xF00)

/* Register type */
enum {
    CH422G_REG_TYPE_WR_SET = 0,
    CH422G_REG_TYPE_WR_OC,
    CH422G_REG_TYPE_WR_IO,
    CH422G_REG_TYPE_RD_IO,
    CH422G_REG_TYPE_NUM,
};

/**
 * @brief Device Structure Type
 */
typedef struct {
    esp_io_expander_t base;
    i2c_master_dev_handle_t i2c_handles[CH422G_REG_TYPE_NUM];
    struct {
        uint8_t wr_set;
        uint8_t wr_oc;
        uint8_t wr_io;
    } regs;
} esp_io_expander_ch422g_t;

static const char *TAG = "ch422g";
static const int   REG_TYPE_ADDR[CH422G_REG_TYPE_NUM] = {
    [CH422G_REG_TYPE_WR_SET] = CH422G_REG_ADDR_WR_SET,
    [CH422G_REG_TYPE_WR_OC]  = CH422G_REG_ADDR_WR_OC,
    [CH422G_REG_TYPE_WR_IO]  = CH422G_REG_ADDR_WR_IO,
    [CH422G_REG_TYPE_RD_IO]  = CH422G_REG_ADDR_RD_IO,
};

static esp_err_t read_input_reg(esp_io_expander_handle_t handle, uint32_t *value);
static esp_err_t write_output_reg(esp_io_expander_handle_t handle, uint32_t value);
static esp_err_t read_output_reg(esp_io_expander_handle_t handle, uint32_t *value);
static esp_err_t write_direction_reg(esp_io_expander_handle_t handle, uint32_t value);
static esp_err_t read_direction_reg(esp_io_expander_handle_t handle, uint32_t *value);
static esp_err_t reset(esp_io_expander_t *handle);
static esp_err_t del(esp_io_expander_t *handle);

esp_err_t esp_io_expander_new_i2c_ch422g(
    i2c_master_bus_handle_t i2c_bus, const i2c_device_config_t *i2c_dev_cfg, esp_io_expander_handle_t *handle_ret
)
{
    ESP_LOGI(TAG, "version: %d.%d.%d", ESP_IO_EXPANDER_CH422G_VER_MAJOR, ESP_IO_EXPANDER_CH422G_VER_MINOR,
             ESP_IO_EXPANDER_CH422G_VER_PATCH);
    ESP_RETURN_ON_FALSE(handle_ret != NULL, ESP_ERR_INVALID_ARG, TAG, "Invalid handle_ret");

    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)calloc(1, sizeof(esp_io_expander_ch422g_t));
    ESP_RETURN_ON_FALSE(ch422g, ESP_ERR_NO_MEM, TAG, "Malloc failed");

    // Add new I2C devices
    esp_err_t ret = ESP_OK;
    i2c_device_config_t temp_cfg = *i2c_dev_cfg;
    for (int i = 0; i < CH422G_REG_TYPE_NUM; i++) {
        temp_cfg.device_address = REG_TYPE_ADDR[i] << 1;
        ret = i2c_master_bus_add_device(i2c_bus, &temp_cfg, &ch422g->i2c_handles[i]);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "Add new I2C device(%d) failed(%s)", REG_TYPE_ADDR[i], esp_err_to_name(ret));
    }

    ch422g->base.config.io_count = IO_COUNT;
    ch422g->regs.wr_set = REG_WR_SET_DEFAULT_VAL;
    ch422g->regs.wr_oc = REG_WR_OC_DEFAULT_VAL;
    ch422g->regs.wr_io = REG_WR_IO_DEFAULT_VAL;
    ch422g->base.read_input_reg = read_input_reg;
    ch422g->base.write_output_reg = write_output_reg;
    ch422g->base.read_output_reg = read_output_reg;
    ch422g->base.write_direction_reg = write_direction_reg;
    ch422g->base.read_direction_reg = read_direction_reg;
    ch422g->base.del = del;
    ch422g->base.reset = reset;

    /* Reset configuration and register status */
    ESP_GOTO_ON_ERROR(reset(&ch422g->base), err, TAG, "Reset failed");

    *handle_ret = &ch422g->base;
    return ESP_OK;

err:
    if (ch422g != NULL) {
        for (int i = 0; i < CH422G_REG_TYPE_NUM; i++) {
            if (ch422g->i2c_handles[i]) {
                i2c_master_bus_rm_device(ch422g->i2c_handles[i]);
            }
        }
        free(ch422g);
    }
    return ret;
}

esp_err_t esp_io_expander_ch422g_set_oc_open_drain(esp_io_expander_handle_t handle)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t data = (uint8_t)(ch422g->regs.wr_set | REG_WR_SET_BIT_OD_EN);

    // WR-SET
    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_SET], &data, sizeof(data), I2C_TIMEOUT_MS),
        TAG, "Write WR_SET reg failed"
    );
    ch422g->regs.wr_set = data;

    return ESP_OK;
}

esp_err_t esp_io_expander_ch422g_set_oc_push_pull(esp_io_expander_handle_t handle)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t data = (uint8_t)(ch422g->regs.wr_set & ~REG_WR_SET_BIT_OD_EN);

    // WR-SET
    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_SET], &data, sizeof(data), I2C_TIMEOUT_MS),
        TAG, "Write WR_SET reg failed"
    );
    ch422g->regs.wr_set = data;

    return ESP_OK;
}

esp_err_t esp_io_expander_ch422g_set_all_input(esp_io_expander_handle_t handle)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t data = (uint8_t)(ch422g->regs.wr_set & ~REG_WR_SET_BIT_IO_OE);

    // WR-SET
    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_SET], &data, sizeof(data), I2C_TIMEOUT_MS),
        TAG, "Write WR_SET reg failed"
    );
    ch422g->regs.wr_set = data;
    // Delay 2ms to wait for the IO expander to switch to input mode
    vTaskDelay(pdMS_TO_TICKS(2));

    return ESP_OK;
}

esp_err_t esp_io_expander_ch422g_set_all_output(esp_io_expander_handle_t handle)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t data = (uint8_t)(ch422g->regs.wr_set | REG_WR_SET_BIT_IO_OE);

    // WR-SET
    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_SET], &data, sizeof(data), I2C_TIMEOUT_MS),
        TAG, "Write WR_SET reg failed"
    );
    ch422g->regs.wr_set = data;

    return ESP_OK;
}

esp_err_t esp_io_expander_ch422g_enter_sleep(esp_io_expander_handle_t handle)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t data = (uint8_t)(ch422g->regs.wr_set | REG_WR_SET_BIT_SLEEP);

    // WR-SET
    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_SET], &data, sizeof(data), I2C_TIMEOUT_MS),
        TAG, "Write WR_SET reg failed"
    );
    ch422g->regs.wr_set = data;

    return ESP_OK;
}

esp_err_t esp_io_expander_ch422g_exit_sleep(esp_io_expander_handle_t handle)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t data = (uint8_t)(ch422g->regs.wr_set & ~REG_WR_SET_BIT_SLEEP);

    // WR-SET
    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_SET], &data, sizeof(data), I2C_TIMEOUT_MS),
        TAG, "Write WR_SET reg failed"
    );
    ch422g->regs.wr_set = data;

    return ESP_OK;
}

static esp_err_t read_input_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t temp = 0;

    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_RD_IO], &temp, sizeof(temp), I2C_TIMEOUT_MS),
        TAG, "Read RD-IO reg failed"
    );
    *value = temp;

    return ESP_OK;
}

static esp_err_t write_output_reg(esp_io_expander_handle_t handle, uint32_t value)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);

    uint8_t wr_oc_data = (value & 0xF00) >> 8;
    uint8_t wr_io_data = value & 0xFF;

    // WR-OC
    if (wr_oc_data) {
        ESP_RETURN_ON_ERROR(
            i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_OC], &wr_oc_data, sizeof(wr_oc_data), I2C_TIMEOUT_MS),
            TAG, "Write WR-OC reg failed"
        );
        ch422g->regs.wr_oc = wr_oc_data;
    }

    // WR-IO
    if (wr_io_data) {
        ESP_RETURN_ON_ERROR(
            i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_IO], &wr_io_data, sizeof(wr_io_data), I2C_TIMEOUT_MS),
            TAG, "Write WR-IO reg failed"
        );
        ch422g->regs.wr_io = wr_io_data;
    }

    return ESP_OK;
}

static esp_err_t read_output_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);

    *value = ch422g->regs.wr_io | (((uint32_t)ch422g->regs.wr_oc) << 8);

    return ESP_OK;
}

static esp_err_t write_direction_reg(esp_io_expander_handle_t handle, uint32_t value)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);
    uint8_t data = ch422g->regs.wr_set;

    value &= 0xFF;
    if (value != 0) {
        data |= REG_WR_SET_BIT_IO_OE;
    } else {
        data &= ~REG_WR_SET_BIT_IO_OE;
    }

    // WR-SET
    ESP_RETURN_ON_ERROR(
        i2c_master_transmit(ch422g->i2c_handles[CH422G_REG_TYPE_WR_SET], &data, sizeof(data), I2C_TIMEOUT_MS), TAG,
        "Write WR_SET reg failed"
    );
    ch422g->regs.wr_set = data;

    return ESP_OK;
}

static esp_err_t read_direction_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);

    *value = (ch422g->regs.wr_set & REG_WR_SET_BIT_IO_OE) ? DIR_OUT_VALUE : DIR_IN_VALUE;

    return ESP_OK;
}

static esp_err_t reset(esp_io_expander_t *handle)
{
    ESP_RETURN_ON_ERROR(write_direction_reg(handle, REG_DIR_DEFAULT_VAL), TAG, "Write direction reg (WR_SET) failed");
    ESP_RETURN_ON_ERROR(write_output_reg(handle, REG_OUT_DEFAULT_VAL), TAG, "Write output reg (WR_OC & WR_IO) failed");

    return ESP_OK;
}

static esp_err_t del(esp_io_expander_t *handle)
{
    esp_io_expander_ch422g_t *ch422g = (esp_io_expander_ch422g_t *)__containerof(handle, esp_io_expander_ch422g_t, base);

    for (int i = 0; i < CH422G_REG_TYPE_NUM; i++) {
        if (ch422g->i2c_handles[i]) {
            i2c_master_bus_rm_device(ch422g->i2c_handles[i]);
        }
    }
    free(ch422g);
    return ESP_OK;
}
