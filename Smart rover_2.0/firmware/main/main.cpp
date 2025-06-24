/**
 * ESP32 Smart Car - Main Application
 * 
 * This file contains the main application code for the ESP32 Smart Car project.
 * It initializes all the required components and sets up the main control loop.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

#include "motor.h"
#include "lights.h"
#include "battery.h"
#include "webserver.h"
#include "failsafe.h"
#include "config.h"

static const char *TAG = "smart_car_main";

// Task handles
TaskHandle_t motorControlTaskHandle = NULL;
TaskHandle_t batteryMonitorTaskHandle = NULL;
TaskHandle_t webServerTaskHandle = NULL;
TaskHandle_t failsafeTaskHandle = NULL;

// System state
SystemState_t systemState;

/**
 * Motor control task - runs on core 1
 * Handles motor control based on commands received from the web interface
 */
void motorControlTask(void *pvParameters) {
    ESP_LOGI(TAG, "Motor control task started on core %d", xPortGetCoreID());
    
    // Initialize motor control
    motor_init();
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(MOTOR_UPDATE_INTERVAL_MS);
    
    while (1) {
        // Update motor speeds based on current control inputs
        motor_update(&systemState);
        
        // Update motor status in system state
        systemState.leftMotorSpeed = motor_get_left_speed();
        systemState.rightMotorSpeed = motor_get_right_speed();
        
        // Delay until the next update
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * Battery monitor task - runs on core 0
 * Monitors battery voltage and updates system state
 */
void batteryMonitorTask(void *pvParameters) {
    ESP_LOGI(TAG, "Battery monitor task started on core %d", xPortGetCoreID());
    
    // Initialize battery monitoring
    battery_init();
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(BATTERY_CHECK_INTERVAL_MS);
    
    while (1) {
        // Update battery status
        float voltage = battery_read_voltage();
        int percentage = battery_calculate_percentage(voltage);
        
        // Update system state
        systemState.batteryVoltage = voltage;
        systemState.batteryPercentage = percentage;
        
        // Check for low battery
        if (voltage < BATTERY_LOW_THRESHOLD) {
            systemState.batteryLow = true;
            ESP_LOGW(TAG, "Low battery warning: %.2fV", voltage);
        } else {
            systemState.batteryLow = false;
        }
        
        // Check for critical battery
        if (voltage < BATTERY_CRITICAL_THRESHOLD) {
            systemState.batteryCritical = true;
            ESP_LOGE(TAG, "Critical battery level: %.2fV", voltage);
            failsafe_trigger(FAILSAFE_BATTERY_CRITICAL);
        } else {
            systemState.batteryCritical = false;
        }
        
        // Delay until the next check
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * Web server task - runs on core 0
 * Handles the web interface and WebSocket communication
 */
void webServerTask(void *pvParameters) {
    ESP_LOGI(TAG, "Web server task started on core %d", xPortGetCoreID());
    
    // Initialize web server
    webserver_init(&systemState);
    
    // This task will be blocked by the web server loop
    webserver_start();
    
    // Should never reach here
    ESP_LOGE(TAG, "Web server task ended unexpectedly");
    vTaskDelete(NULL);
}

/**
 * Failsafe task - runs on core 1
 * Monitors system health and triggers failsafe actions when needed
 */
void failsafeTask(void *pvParameters) {
    ESP_LOGI(TAG, "Failsafe task started on core %d", xPortGetCoreID());
    
    // Initialize failsafe system
    failsafe_init(&systemState);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(FAILSAFE_CHECK_INTERVAL_MS);
    
    while (1) {
        // Check for failsafe conditions
        failsafe_check();
        
        // Update system state
        systemState.failsafeActive = failsafe_is_active();
        systemState.failsafeReason = failsafe_get_reason();
        
        // Delay until the next check
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * Initialize SPIFFS for serving web files
 */
void init_spiffs() {
    ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

/**
 * Initialize Wi-Fi in Access Point mode
 */
void init_wifi_ap() {
    ESP_LOGI(TAG, "Initializing WiFi in AP mode");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize TCP/IP adapter
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Create default AP
    esp_netif_create_default_wifi_ap();
    
    // Initialize Wi-Fi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Configure AP
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASSWORD,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    
    // Set empty password for open network
    if (strlen(WIFI_AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    
    // Configure and start AP
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi AP started with SSID: %s, password: %s", WIFI_AP_SSID, WIFI_AP_PASSWORD);
}

/**
 * Initialize the lighting system
 */
void init_lights() {
    ESP_LOGI(TAG, "Initializing lighting system");
    
    // Initialize the lighting system
    lights_init();
    
    // Set initial light states
    lights_set_headlights(false);
    lights_set_taillights(false);
    lights_set_indicators(INDICATOR_NONE);
    lights_set_underglow(0, 0, 0); // RGB off
}

/**
 * Application main entry point
 */
extern "C" void app_main() {
    ESP_LOGI(TAG, "ESP32 Smart Car starting up");
    
    // Initialize system state
    memset(&systemState, 0, sizeof(SystemState_t));
    systemState.isInitialized = false;
    
    // Initialize components
    init_spiffs();
    init_wifi_ap();
    init_lights();
    
    // Create tasks
    // Motor control task on core 1 (high priority)
    xTaskCreatePinnedToCore(
        motorControlTask,
        "MotorControl",
        4096,
        NULL,
        5,
        &motorControlTaskHandle,
        1);
    
    // Battery monitor task on core 0 (medium priority)
    xTaskCreatePinnedToCore(
        batteryMonitorTask,
        "BatteryMonitor",
        2048,
        NULL,
        3,
        &batteryMonitorTaskHandle,
        0);
    
    // Failsafe task on core 1 (highest priority)
    xTaskCreatePinnedToCore(
        failsafeTask,
        "Failsafe",
        2048,
        NULL,
        6,
        &failsafeTaskHandle,
        1);
    
    // Web server task on core 0 (medium priority)
    xTaskCreatePinnedToCore(
        webServerTask,
        "WebServer",
        8192, // Larger stack for web server
        NULL,
        4,
        &webServerTaskHandle,
        0);
    
    // System is now initialized
    systemState.isInitialized = true;
    ESP_LOGI(TAG, "System initialization complete");
    
    // Flash lights to indicate successful startup
    lights_set_headlights(true);
    vTaskDelay(pdMS_TO_TICKS(500));
    lights_set_headlights(false);
    vTaskDelay(pdMS_TO_TICKS(500));
    lights_set_headlights(true);
    vTaskDelay(pdMS_TO_TICKS(500));
    lights_set_headlights(false);
    
    // Main task can now be deleted as all work is done in other tasks
    vTaskDelete(NULL);
}