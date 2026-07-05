#include <stdio.h>
#include <string.h>
#include <inttypes.h>

extern "C" {
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"

// HID Device API (Bluedroid Classic HID Device profile)
#include "esp_hidd_api.h"
}

static const char *TAG = "CLASSIC_HID_SKELETON";

static volatile bool g_hid_initialized = false;
static volatile bool g_hid_app_registered = false;
static volatile bool g_hid_connected = false;
static esp_bd_addr_t g_peer_bda = {0};

/**
 * Placeholder HID report descriptor.
 * Replace with a real gamepad descriptor during implementation phase.
 */
static uint8_t hid_report_descriptor[] = {
    // Usage Page (Generic Desktop), Usage (Gamepad), Collection (Application)
    0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,       // USAGE (Gamepad)
    0xA1, 0x01,       // COLLECTION (Application)
    0x15, 0x00,       //   LOGICAL_MINIMUM (0)
    0x25, 0x01,       //   LOGICAL_MAXIMUM (1)
    0x35, 0x00,       //   PHYSICAL_MINIMUM (0)
    0x45, 0x01,       //   PHYSICAL_MAXIMUM (1)
    0x75, 0x01,       //   REPORT_SIZE (1)
    0x95, 0x10,       //   REPORT_COUNT (16) -- placeholder buttons
    0x05, 0x09,       //   USAGE_PAGE (Button)
    0x19, 0x01,       //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,       //   USAGE_MAXIMUM (Button 16)
    0x81, 0x02,       //   INPUT (Data,Var,Abs)
    0x75, 0x08,       //   REPORT_SIZE (8)
    0x95, 0x04,       //   REPORT_COUNT (4) -- placeholder axes
    0x05, 0x01,       //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,       //   USAGE (X)
    0x09, 0x31,       //   USAGE (Y)
    0x09, 0x32,       //   USAGE (Z)
    0x09, 0x35,       //   USAGE (Rz)
    0x15, 0x81,       //   LOGICAL_MINIMUM (-127)
    0x25, 0x7F,       //   LOGICAL_MAXIMUM (127)
    0x81, 0x02,       //   INPUT (Data,Var,Abs)
    0xC0              // END_COLLECTION
};

static void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            ESP_LOGI(TAG, "GAP auth complete, stat=%d", param->auth_cmpl.stat);
            break;
        case ESP_BT_GAP_PIN_REQ_EVT:
            ESP_LOGI(TAG, "GAP PIN request (legacy pairing)");
            break;
        case ESP_BT_GAP_CFM_REQ_EVT:
            ESP_LOGI(TAG, "GAP SSP confirm request: num_val=%lu",
                     (unsigned long)param->cfm_req.num_val);
            // Auto-confirm for bring-up; tighten for production if required.
            esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
            break;
        case ESP_BT_GAP_MODE_CHG_EVT:
            ESP_LOGI(TAG, "GAP mode changed: mode=%d", param->mode_chg.mode);
            break;
        default:
            ESP_LOGD(TAG, "GAP event: %d", event);
            break;
    }
}

static void hidd_event_callback(esp_hidd_cb_event_t event,
                                esp_hidd_cb_param_t *param) {
    ESP_LOGI(TAG, "HID event received: %d", (int)event);

    switch (event) {
        case ESP_HIDD_INIT_EVT:
            if (param && param->init.status == ESP_HIDD_SUCCESS) {
                g_hid_initialized = true;
                ESP_LOGI(TAG, "HID init success");
            } else {
                ESP_LOGE(TAG, "HID init failed");
            }
            break;

        case ESP_HIDD_REGISTER_APP_EVT:
            if (param && param->register_app.status == ESP_HIDD_SUCCESS) {
                g_hid_app_registered = true;
                ESP_LOGI(TAG, "HID app registered");
            } else {
                ESP_LOGE(TAG, "HID app register failed");
            }
            break;

        case ESP_HIDD_OPEN_EVT:
            g_hid_connected = true;
            if (param) {
                memcpy(g_peer_bda, param->open.bd_addr, sizeof(esp_bd_addr_t));
            }
            ESP_LOGI(TAG, "HID open (connected)");
            break;

        case ESP_HIDD_CLOSE_EVT:
            g_hid_connected = false;
            ESP_LOGW(TAG, "HID close (disconnected)");
            break;

        case ESP_HIDD_SET_PROTOCOL_EVT:
            if (param) {
                ESP_LOGI(TAG, "SET_PROTOCOL mode=%d", (int)param->set_protocol.protocol_mode);
            }
            break;

        case ESP_HIDD_GET_REPORT_EVT:
            if (param) {
                ESP_LOGI(TAG, "GET_REPORT type=%d id=%d len=%d",
                         (int)param->get_report.report_type,
                         (int)param->get_report.report_id,
                         (int)param->get_report.buffer_size);

                // Reply with a minimal neutral report (all zeros).
                uint8_t neutral_report[8] = {0};
                esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 1, sizeof(neutral_report), neutral_report);
            }
            break;

        case ESP_HIDD_SET_REPORT_EVT:
            if (param) {
                ESP_LOGI(TAG, "SET_REPORT type=%d id=%d len=%d",
                         (int)param->set_report.report_type,
                         (int)param->set_report.report_id,
                         (int)param->set_report.len);
            }
            break;

        default:
            break;
    }
}

static esp_err_t init_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting ESP32 Classic BT HID skeleton...");

    ESP_ERROR_CHECK(init_nvs());

    // Release BLE memory since we target Classic BR/EDR HID.
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_bt_gap_register_callback(gap_callback));

    // Device identity for discovery (name can be adjusted for test strategy).
    ESP_ERROR_CHECK(esp_bt_gap_set_device_name("Xbox Wireless Controller"));

    // Make device discoverable/connectable.
    ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));

    // HID Device registration config
    esp_hidd_app_param_t app_param = {
        .name = "Xbox Wireless Controller",
        .description = "Classic HID Gamepad Skeleton",
        .provider = "Microsoft",
        .subclass = ESP_HID_CLASS_MIC,
        .desc_list = hid_report_descriptor,
        .desc_list_len = sizeof(hid_report_descriptor),
    };

    esp_hidd_qos_param_t both_qos = {
        .service_type = 1, // Best effort
        .token_rate = 0,
        .token_bucket_size = 0,
        .peak_bandwidth = 0,
        .access_latency = 0xFFFFFFFF,
        .delay_variation = 0xFFFFFFFF,
    };

    ESP_ERROR_CHECK(esp_bt_hid_device_register_callback(hidd_event_callback));
    ESP_ERROR_CHECK(esp_bt_hid_device_init());

    // Event-driven sequencing: register app only after init callback confirms readiness.
    int init_wait_ms = 0;
    while (!g_hid_initialized && init_wait_ms < 10000) {
        vTaskDelay(pdMS_TO_TICKS(50));
        init_wait_ms += 50;
    }
    ESP_ERROR_CHECK(g_hid_initialized ? ESP_OK : ESP_FAIL);

    ESP_ERROR_CHECK(esp_bt_hid_device_register_app(&app_param, &both_qos, &both_qos));

    int app_wait_ms = 0;
    while (!g_hid_app_registered && app_wait_ms < 10000) {
        vTaskDelay(pdMS_TO_TICKS(50));
        app_wait_ms += 50;
    }
    ESP_ERROR_CHECK(g_hid_app_registered ? ESP_OK : ESP_FAIL);

    ESP_LOGI(TAG, "HID stack initialized and app registered; entering idle loop.");

    // Minimal idle report task behavior:
    // send neutral report periodically only when connected.
    uint8_t neutral_report[8] = {0};

    while (true) {
        if (g_hid_connected) {
            esp_err_t r = esp_bt_hid_device_send_report(
                ESP_HIDD_REPORT_TYPE_INTRDATA, 1, sizeof(neutral_report), neutral_report
            );
            if (r != ESP_OK) {
                ESP_LOGW(TAG, "send_report failed: %s", esp_err_to_name(r));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
