/**
 * @file app_priv.h
 * IO Connect - Standard devices - Codor Stelian <codor.stelian.n@gmail.com>
 * NO LICENSE
 */
#pragma once

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_POWER  true
extern esp_rmaker_device_t *switch_device;
void app_driver_init(void);
int app_driver_set_state(bool state);
bool app_driver_get_state(void);