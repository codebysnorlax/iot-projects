/**
 * ESP32 Smart Car - Battery Monitoring Implementation
 * 
 * This file implements the battery monitoring functionality for the ESP32 Smart Car.
 * It handles battery voltage measurement and state of charge estimation.
 */

#include "battery.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "battery";

// ADC calibration
static esp_adc_cal_characteristics_t adc_chars;

// Battery state
typedef struct {
    float dividerRatio;     // Voltage divider ratio (Vbat / Vadc)
    float lastVoltage;      // Last measured voltage
    int lastPercentage;     // Last calculated percentage
    bool isCalibrated;      // Whether the ADC has been calibrated
} BatteryState_t;

static BatteryState_t batteryState = {0};

// NVS keys
#define NVS_NAMESPACE "battery"
#define NVS_KEY_DIVIDER_RATIO "div_ratio"

/**
 * Initialize the battery monitoring module
 */
void battery_init(void) {
    ESP_LOGI(TAG, "Initializing battery monitoring");
    
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO34 is ADC1_CHANNEL_6
    
    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, BATTERY_ADC_VREF, &adc_chars);
    
    // Calculate default divider ratio
    float defaultRatio = (float)(BATTERY_R1 + BATTERY_R2) / BATTERY_R2;
    
    // Try to load divider ratio from NVS
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    
    if (err == ESP_OK) {
        float storedRatio;
        size_t required_size = sizeof(float);
        err = nvs_get_blob(nvs_handle, NVS_KEY_DIVIDER_RATIO, &storedRatio, &required_size);
        
        if (err == ESP_OK) {
            batteryState.dividerRatio = storedRatio;
            batteryState.isCalibrated = true;
            ESP_LOGI(TAG, "Loaded calibrated divider ratio: %.4f", storedRatio);
        } else {
            batteryState.dividerRatio = defaultRatio;
            batteryState.isCalibrated = false;
            ESP_LOGI(TAG, "Using default divider ratio: %.4f", defaultRatio);
        }
        
        nvs_close(nvs_handle);
    } else {
        batteryState.dividerRatio = defaultRatio;
        batteryState.isCalibrated = false;
        ESP_LOGI(TAG, "Using default divider ratio: %.4f", defaultRatio);
    }
    
    // Initialize battery state
    batteryState.lastVoltage = 0.0f;
    batteryState.lastPercentage = 0;
    
    // Take an initial reading
    float voltage = battery_read_voltage();
    int percentage = battery_calculate_percentage(voltage);
    
    ESP_LOGI(TAG, "Initial battery reading: %.2fV (%d%%)", voltage, percentage);
}

/**
 * Read the current battery voltage
 */
float battery_read_voltage(void) {
    uint32_t adc_reading = 0;
    
    // Sample ADC multiple times for better accuracy
    for (int i = 0; i < BATTERY_ADC_SAMPLES; i++) {
        adc_reading += adc1_get_raw(ADC1_CHANNEL_6);
    }
    
    adc_reading /= BATTERY_ADC_SAMPLES;
    
    // Convert ADC reading to voltage
    uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    
    // Convert to battery voltage using divider ratio
    float battery_voltage = (voltage_mv / 1000.0f) * batteryState.dividerRatio;
    
    // Update last voltage
    batteryState.lastVoltage = battery_voltage;
    
    return battery_voltage;
}

/**
 * Calculate battery percentage based on voltage
 */
int battery_calculate_percentage(float voltage) {
    // Calculate percentage based on voltage range
    float percentage = (voltage - BATTERY_EMPTY_VOLTAGE) / 
                      (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE) * 100.0f;
    
    // Clamp to valid range
    if (percentage > 100.0f) percentage = 100.0f;
    if (percentage < 0.0f) percentage = 0.0f;
    
    // Update last percentage
    batteryState.lastPercentage = (int)percentage;
    
    return (int)percentage;
}

/**
 * Check if battery is low
 */
bool battery_is_low(void) {
    return batteryState.lastVoltage < BATTERY_LOW_THRESHOLD;
}

/**
 * Check if battery is critically low
 */
bool battery_is_critical(void) {
    return batteryState.lastVoltage < BATTERY_CRITICAL_THRESHOLD;
}

/**
 * Calibrate the battery voltage measurement
 */
bool battery_calibrate(float measured_voltage) {
    // Ensure we have a valid measured voltage
    if (measured_voltage <= 0.0f) {
        ESP_LOGE(TAG, "Invalid measured voltage for calibration: %.2fV", measured_voltage);
        return false;
    }
    
    // Read ADC value
    uint32_t adc_reading = 0;
    
    // Sample ADC multiple times for better accuracy
    for (int i = 0; i < BATTERY_ADC_SAMPLES * 2; i++) {
        adc_reading += adc1_get_raw(ADC1_CHANNEL_6);
    }
    
    adc_reading /= (BATTERY_ADC_SAMPLES * 2);
    
    // Convert ADC reading to voltage
    uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    float adc_voltage = voltage_mv / 1000.0f;
    
    // Calculate new divider ratio
    float new_ratio = measured_voltage / adc_voltage;
    
    // Validate the new ratio
    if (new_ratio <= 1.0f || new_ratio > 10.0f) {
        ESP_LOGE(TAG, "Calculated divider ratio out of reasonable range: %.4f", new_ratio);
        return false;
    }
    
    ESP_LOGI(TAG, "Calibration: ADC=%.3fV, Measured=%.3fV, New Ratio=%.4f", 
             adc_voltage, measured_voltage, new_ratio);
    
    // Save the new ratio
    batteryState.dividerRatio = new_ratio;
    batteryState.isCalibrated = true;
    
    // Save to NVS
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    
    if (err == ESP_OK) {
        err = nvs_set_blob(nvs_handle, NVS_KEY_DIVIDER_RATIO, &new_ratio, sizeof(float));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error saving divider ratio to NVS: %s", esp_err_to_name(err));
        }
        
        err = nvs_commit(nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        }
        
        nvs_close(nvs_handle);
    } else {
        ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(err));
        return false;
    }
    
    return true;
}

/**
 * Get the battery voltage divider ratio
 */
float battery_get_divider_ratio(void) {
    return batteryState.dividerRatio;
}

/**
 * Set the battery voltage divider ratio
 */
void battery_set_divider_ratio(float ratio) {
    if (ratio <= 1.0f || ratio > 10.0f) {
        ESP_LOGE(TAG, "Invalid divider ratio: %.4f", ratio);
        return;
    }
    
    batteryState.dividerRatio = ratio;
    
    // Save to NVS
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    
    if (err == ESP_OK) {
        err = nvs_set_blob(nvs_handle, NVS_KEY_DIVIDER_RATIO, &ratio, sizeof(float));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error saving divider ratio to NVS: %s", esp_err_to_name(err));
        }
        
        err = nvs_commit(nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        }
        
        nvs_close(nvs_handle);
    } else {
        ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(err));
    }
}