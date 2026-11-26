/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_idf_version.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
#error "ESP32_IO_Expander 2.x requires ESP-IDF 5.4 or Arduino-ESP32 3.2.0 or later."
#endif

/* Porting drivers */
#include "port/esp_io_expander.h"
#include "port/esp_io_expander_ch422g.h"
#include "port/esp_io_expander_ht8574.h"
#include "port/esp_io_expander_tca9554.h"
#include "port/esp_io_expander_tca95xx_16bit.h"

/* Wrapper classes */
#include "esp_expander_base.hpp"
#include "esp_expander_ch422g.hpp"
#include "esp_expander_ht8574.hpp"
#include "esp_expander_tca95xx_8bit.hpp"
#include "esp_expander_tca95xx_16bit.hpp"
