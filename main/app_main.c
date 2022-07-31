/*
 * BSD 2-Clause License
 * 
 * Copyright (c) 2021, Codor Stelian <codor.stelian.n@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <esp_log.h>
#include <esp_event.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_common_events.h>
#include <esp_rmaker_console.h>
#include <esp_rmaker_utils.h>
#include <esp_wifi.h>
#include <ioc_standard_devices.h>
#include <ioc_standard_params.h>
#include <ioc_standard_types.h>
#include <wifi_provisioning/manager.h>

#include <nvs_flash.h>
#include <string.h>

#include <app_insights.h>
#include <app_priv.h>
#include <app_rainmaker.h>
#include <app_wifi.h>
#include <wifi_reconnect.h>

static const char TAG[] = "app_main";
static const char TAG_EVENT[] = "app_main_EVENT";

// Devices
esp_rmaker_device_t *outlet_device;
esp_rmaker_device_t *switch_device;
esp_rmaker_device_t *lightbulb;
esp_rmaker_device_t *lightbulb_cct;
esp_rmaker_device_t *lightbulb_rgb;
esp_rmaker_device_t *lock_device;
esp_rmaker_device_t *thermostat_device;
esp_rmaker_device_t *fan_device;
esp_rmaker_device_t *temperature_sensor;
esp_rmaker_device_t *humidity_sensor;
esp_rmaker_device_t *luminosity_sensor;
esp_rmaker_device_t *esp_device;

app_wifi_pop_type_t pop_type = POP_TYPE_MAC;

#define APP_DEVICE_NAME CONFIG_APP_DEVICE_NAME
#define APP_DEVICE_TYPE CONFIG_APP_DEVICE_TYPE

// Program
static void app_devices_init(esp_rmaker_node_t *node);

// Callback to handle commands received from the RainMaker cloud
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);
    if (ctx)
    {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
	else if (strcmp(device_name, "Switch") == 0)
    {
        if (strcmp(param_name, IOC_DEF_POWER_NAME) == 0)
        {
			ESP_LOGI(TAG, "Received value = %s for %s - %s", val.val.b ? "true" : "false", device_name, param_name);
			app_driver_set_state(val.val.b);
        }
	}
    else if (strcmp(device_name, "Light") == 0)
    {
        if (strcmp(param_name, IOC_DEF_POWER_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %s for %s - %s", val.val.b ? "true" : "false", device_name, param_name);
        }
        else if (strcmp(param_name, IOC_DEF_BRIGHTNESS_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %d for %s - %s", val.val.i, device_name, param_name);
        }
    }
    else if (strcmp(device_name, "CCT Light") == 0)
    {
        if (strcmp(param_name, IOC_DEF_POWER_NAME) == 0)
        {
			ESP_LOGI(TAG, "Received value = %s for %s - %s", val.val.b ? "true" : "false", device_name, param_name);
        }
		else if (strcmp(param_name, IOC_DEF_CCT_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %d for %s - %s", val.val.i, device_name, param_name);
        }
		else if (strcmp(param_name, IOC_DEF_INTENSITY_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %d for %s - %s", val.val.i, device_name, param_name);
        }
    }
    else if (strcmp(device_name, "RGB Light") == 0)
    {
        if (strcmp(param_name, IOC_DEF_POWER_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %s for %s - %s", val.val.b ? "true" : "false", device_name, param_name);
        }
        else if (strcmp(param_name, IOC_DEF_BRIGHTNESS_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %d for %s - %s", val.val.i, device_name, param_name);
        }
        else if (strcmp(param_name, IOC_DEF_HUE_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %d for %s - %s", val.val.i, device_name, param_name);
        }
        else if (strcmp(param_name, IOC_DEF_SATURATION_NAME) == 0)
        {
            ESP_LOGI(TAG, "Received value = %d for %s - %s", val.val.i, device_name, param_name);
        }
    }
    else if (strcmp(device_name, "ESP Device") == 0)
    {
        ESP_LOGI(TAG, "Received value = %s for %s - %s", val.val.b ? "true" : "false", device_name, param_name);
        if (strcmp(param_name, IOC_DEF_REBOOT_NAME) == 0)
        {
            esp_rmaker_reboot(10);
        }
        else if (strcmp(param_name, IOC_DEF_WIFI_RESET_NAME) == 0)
        {
            esp_rmaker_wifi_reset(5, 10);
        }
        else if (strcmp(param_name, IOC_DEF_FACTORY_RESET_NAME) == 0)
        {
            esp_rmaker_factory_reset(5, 10);
        }
    }
    else
    {
        // Silently ignoring invalid params
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}

// Event handler for catching Wi-Fi events
static void event_handler(void* arg, esp_event_base_t event_base,
                          int event_id, void* event_data)
{
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG_EVENT, "Provisioning started");
                break;
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG_EVENT, "Provisioning successful");
                break;
            default:
                break;
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG_EVENT, "WiFi disconnected");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG_EVENT, "WiFi connected");
    }
}

// Event handler for catching RainMaker events
static void rmk_event_handler(void* arg, esp_event_base_t event_base,
                          int event_id, void* event_data)
{
    if (event_base == RMAKER_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_INIT_DONE:
                ESP_LOGI(TAG_EVENT, "RainMaker Initialised.");
                break;
            case RMAKER_EVENT_CLAIM_STARTED:
                ESP_LOGI(TAG_EVENT, "RainMaker Claim Started.");
                break;
            case RMAKER_EVENT_CLAIM_SUCCESSFUL:
                ESP_LOGI(TAG_EVENT, "RainMaker Claim Successful.");
                break;
            case RMAKER_EVENT_CLAIM_FAILED:
                ESP_LOGI(TAG_EVENT, "RainMaker Claim Failed.");
                break;
            default:
                ESP_LOGW(TAG_EVENT, "Unhandled RainMaker Event: %d", event_id);
        }
	} else if (event_base == APP_WIFI_EVENT) {
        switch (event_id) {
            case APP_WIFI_EVENT_QR_DISPLAY:
                ESP_LOGI(TAG_EVENT, "Provisioning QR : %s", (char *)event_data);
                break;
            case APP_WIFI_EVENT_PROV_TIMEOUT:
                ESP_LOGI(TAG_EVENT, "Provisioning Timed Out. Please reboot.");
                break;
            case APP_WIFI_EVENT_PROV_RESTART:
                ESP_LOGI(TAG_EVENT, "Provisioning has restarted due to failures.");
                break;
            default:
                ESP_LOGW(TAG_EVENT, "Unhandled App Wi-Fi Event: %d", event_id);
                break;
        }
    } else if (event_base == RMAKER_COMMON_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_REBOOT:
                ESP_LOGI(TAG_EVENT, "Rebooting in %d seconds.", *((uint8_t *)event_data));
                break;
            case RMAKER_EVENT_WIFI_RESET:
                ESP_LOGI(TAG_EVENT, "Wi-Fi credentials reset.");
                break;
            case RMAKER_EVENT_FACTORY_RESET:
                ESP_LOGI(TAG_EVENT, "Node reset to factory defaults.");
                break;
            case RMAKER_MQTT_EVENT_CONNECTED:
                ESP_LOGI(TAG_EVENT, "MQTT Connected.");
                break;
            case RMAKER_MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG_EVENT, "MQTT Disconnected.");
                break;
            case RMAKER_MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG_EVENT, "MQTT Published. Msg id: %d.", *((int *)event_data));
                break;
            default:
                ESP_LOGW(TAG_EVENT, "Unhandled RainMaker Common Event: %d", event_id);
        }
    } else {
        ESP_LOGW(TAG_EVENT, "Invalid event received!");
    }
}

void setup()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Setup
	esp_rmaker_console_init();
    app_driver_init();

    // Wi-Fi
    struct app_wifi_config wifi_cfg = {
        .wifi_connect = wifi_reconnect_resume,
    };
    ESP_ERROR_CHECK(app_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MAX_MODEM));

    // Register an event handler to catch Wi-Fi, IP and Provisioning events
    err = esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    err = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL);
    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Could not register Wi-Fi event handler");
    }

    // Start Wi-Fi reconnect
    ESP_ERROR_CHECK(wifi_reconnect_start());

    // Register an event handler to catch RainMaker events
    err = esp_event_handler_register(RMAKER_EVENT, ESP_EVENT_ANY_ID, &rmk_event_handler, NULL);
    err = esp_event_handler_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID, &rmk_event_handler, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Could not register RainMaker event handler");
    }

    // RainMaker
    esp_rmaker_node_t *node = NULL;
    ESP_ERROR_CHECK(app_rmaker_init(CONFIG_APP_DEVICE_NAME, CONFIG_APP_DEVICE_TYPE, &node));

    // Devices
    app_devices_init(node);

    // Enable Insights
    app_insights_enable();

    // Start
    ESP_ERROR_CHECK(esp_rmaker_start());

    // Start the Wi-Fi.
    // If the node is provisioned, it will start connection attempts,
    // else, it will start Wi-Fi provisioning. The function will return
    // after a connection has been successfully established
    err = app_wifi_start(pop_type);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not start WiFi!");
    }

    // Done
    ESP_LOGI(TAG, "Setup complete");
}

static void app_devices_init(esp_rmaker_node_t *node)
{
	/* Create a Switch device.
     * You can optionally use the helper API esp_rmaker_switch_device_create() to
     * avoid writing code for adding the name and power parameters.
     */
    switch_device = esp_rmaker_device_create("Switch", IOC_DEVICE_SWITCH, NULL);

    /* Add the write callback for the device. We aren't registering any read callback yet as
     * it is for future use.
     */
    esp_rmaker_device_add_cb(switch_device, write_cb, NULL);

    /* Add the standard name parameter (type: esp.param.name), which allows setting a persistent,
     * user friendly custom name from the phone apps. All devices are recommended to have this
     * parameter.
     */
    esp_rmaker_device_add_param(switch_device, ioc_name_param_create(IOC_DEF_NAME_PARAM, "Switch"));

    /* Add the standard power_btn parameter (type: esp.param.power), which adds a boolean param
     * with a push btn big ui-type.
     */
    esp_rmaker_param_t *power_param = ioc_power_btn_param_create(IOC_DEF_POWER_NAME, DEFAULT_POWER);
    esp_rmaker_device_add_param(switch_device, power_param);

    /* Assign the power parameter as the primary, so that it can be controlled from the
     * home screen of the phone apps.
     */
    esp_rmaker_device_assign_primary_param(switch_device, power_param);

    /* Add this switch device to the node */
    esp_rmaker_node_add_device(node, switch_device);
	
	// Create all ioc standard devices and params
	outlet_device = ioc_outlet_device_create("Power plug", NULL, true);
    esp_rmaker_node_add_device(node, outlet_device);
    lightbulb = ioc_lightbulb_device_create("Light", NULL, false);
    esp_rmaker_device_add_cb(lightbulb, write_cb, NULL);
    esp_rmaker_device_add_param(lightbulb, ioc_brightness_param_create(IOC_DEF_BRIGHTNESS_NAME, 15));
    esp_rmaker_device_add_param(lightbulb, ioc_intensity_param_create(IOC_DEF_INTENSITY_NAME, 25));
    esp_rmaker_node_add_device(node, lightbulb);
	
	lightbulb_cct = ioc_lightbulb_cct_device_create("CCT Light", NULL, false);
    esp_rmaker_device_add_cb(lightbulb_cct, write_cb, NULL);
    esp_rmaker_device_add_param(lightbulb_cct, ioc_brightness_param_create(IOC_DEF_BRIGHTNESS_NAME, 15));
    esp_rmaker_device_add_param(lightbulb_cct, ioc_cct_param_create(IOC_DEF_CCT_NAME, 4200));
	esp_rmaker_device_add_param(lightbulb_cct, ioc_hue_param_create(IOC_DEF_HUE_NAME, 90));
    esp_rmaker_node_add_device(node, lightbulb_cct);

    lightbulb_rgb = ioc_lightbulb_rgb_device_create("RGB Light", NULL, false);
    esp_rmaker_device_add_cb(lightbulb_rgb, write_cb, NULL);
    esp_rmaker_device_add_param(lightbulb_rgb, ioc_hue_circle_param_create(IOC_DEF_HUE_NAME, 10));
    esp_rmaker_device_add_param(lightbulb_rgb, ioc_saturation_param_create(IOC_DEF_SATURATION_NAME, 100));
    esp_rmaker_device_add_param(lightbulb_rgb, ioc_brightness_param_create(IOC_DEF_BRIGHTNESS_NAME, 30));
    esp_rmaker_node_add_device(node, lightbulb_rgb);
	
	lock_device = ioc_lock_device_create("Lock Device", NULL, false);
    esp_rmaker_node_add_device(node, lock_device);
	
	thermostat_device = ioc_thermostat_device_create("Thermostat", NULL, 21);
	esp_rmaker_param_t *term_float = esp_rmaker_param_create(IOC_DEF_SET_TEMPERATURE_NAME, IOC_PARAM_TEMPERATURE,
            esp_rmaker_float(25.5), PROP_FLAG_READ | PROP_FLAG_WRITE);
	esp_rmaker_param_add_ui_type(term_float, IOC_UI_TYPE_KNOB_BTN);
	esp_rmaker_param_add_bounds(term_float, esp_rmaker_float(15), esp_rmaker_float(30), esp_rmaker_float(1));
	esp_rmaker_device_add_param(thermostat_device, term_float);
    esp_rmaker_node_add_device(node, thermostat_device);
	
	fan_device = ioc_fan_device_create("Fan", NULL, false);
    esp_rmaker_device_add_param(fan_device, ioc_speed_param_create(IOC_DEF_SPEED_NAME, 3));
    esp_rmaker_device_add_param(fan_device, ioc_direction_param_create(IOC_DEF_DIRECTION_NAME, 1));
    esp_rmaker_node_add_device(node, fan_device);

    temperature_sensor = ioc_temp_sensor_device_create("Temperature Sensor", NULL, 24.3);
    esp_rmaker_node_add_device(node, temperature_sensor);

    humidity_sensor = ioc_humid_sensor_device_create("Humidity Sensor", NULL, 60.5);
    esp_rmaker_node_add_device(node, humidity_sensor);

    luminosity_sensor = ioc_lumen_sensor_device_create("Luminosity Sensor", NULL, 155);
    esp_rmaker_node_add_device(node, luminosity_sensor);

    esp_device = esp_rmaker_device_create("ESP Device", NULL, NULL);
    esp_rmaker_device_add_cb(esp_device, write_cb, NULL);
    esp_rmaker_device_add_param(esp_device, ioc_name_param_create(IOC_DEF_NAME_PARAM, "ESP Device"));
    char mac[18];
    esp_err_t err = get_dev_mac(mac);
    if (err == ESP_OK && mac != NULL) {
        esp_rmaker_device_add_attribute(esp_device, "MAC", mac);
        ESP_LOGI(TAG, "MAC address: %s", mac);
    }
    char pop[9];
    err = get_dev_pop(pop, pop_type);
    if (err == ESP_OK && pop != NULL) {
        esp_rmaker_device_add_attribute(esp_device, "PoP", pop);
    }
    esp_rmaker_device_add_param(esp_device, ioc_reboot_param_create(IOC_DEF_REBOOT_NAME));
    esp_rmaker_device_add_param(esp_device, ioc_wifi_reset_param_create(IOC_DEF_WIFI_RESET_NAME));
    esp_rmaker_device_add_param(esp_device, ioc_factory_reset_param_create(IOC_DEF_FACTORY_RESET_NAME));
    esp_rmaker_node_add_device(node, esp_device);
}

void app_main()
{
    setup();

    //Add code to run
}