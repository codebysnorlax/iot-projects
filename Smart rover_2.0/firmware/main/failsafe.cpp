/**
 * ESP32 Smart Car - Failsafe System Implementation
 * 
 * This file implements the failsafe system functionality for the ESP32 Smart Car.
 * It handles safety mechanisms like timeout detection, overcurrent protection, and motor blocking.
 */

#include "failsafe.h"
#include "motor.h"
#include "lights.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "failsafe";

// Failsafe state
typedef struct {
    SystemState_t *systemState;  // Pointer to system state
    bool isActive;               // Whether failsafe is currently active
    uint8_t reason;              // Reason for failsafe activation
    uint32_t activationTime;     // Time when failsafe was activated
    uint32_t lastCommandTime;    // Time of last valid command
} FailsafeState_t;

static FailsafeState_t failsafeState = {0};

/**
 * Initialize the failsafe system
 */
void failsafe_init(SystemState_t *state) {
    ESP_LOGI(TAG, "Initializing failsafe system");
    
    // Store pointer to system state
    failsafeState.systemState = state;
    
    // Initialize failsafe state
    failsafeState.isActive = false;
    failsafeState.reason = FAILSAFE_NONE;
    failsafeState.activationTime = 0;
    failsafeState.lastCommandTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    ESP_LOGI(TAG, "Failsafe system initialized");
}

/**
 * Check for failsafe conditions
 */
void failsafe_check(void) {
    uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // If failsafe is already active, check if recovery time has passed
    if (failsafeState.isActive) {
        if (currentTime - failsafeState.activationTime >= FAILSAFE_RECOVERY_TIME) {
            // Attempt to recover from failsafe
            if (failsafeState.reason != FAILSAFE_BATTERY_CRITICAL) {
                ESP_LOGI(TAG, "Attempting to recover from failsafe");
                failsafe_reset();
            }
        }
        return;
    }
    
    // Check for communication timeout
    if (currentTime - failsafeState.lastCommandTime >= FAILSAFE_TIMEOUT_MS) {
        ESP_LOGW(TAG, "Communication timeout detected");
        failsafe_trigger(FAILSAFE_TIMEOUT);
        return;
    }
    
    // Check for battery critical condition
    if (failsafeState.systemState->batteryCritical) {
        ESP_LOGW(TAG, "Critical battery level detected");
        failsafe_trigger(FAILSAFE_BATTERY_CRITICAL);
        return;
    }
    
    // Check for motor blocked condition (simplified implementation)
    // In a real implementation, you would monitor motor current or encoder feedback
    // to detect if a motor is blocked while power is applied
    bool motorsRunning = !motor_is_stopped();
    bool motorCommandsActive = (failsafeState.systemState->leftMotorCommand != 0 || 
                              failsafeState.systemState->rightMotorCommand != 0);
    
    // This is a very simplified check - in a real implementation you would need
    // more sophisticated detection based on current sensing or encoder feedback
    if (motorsRunning && motorCommandsActive) {
        // Check for motor blocking would go here
        // For now, we'll just assume motors are not blocked
    }
    
    // Check for overcurrent condition
    // In a real implementation, you would read from a current sensor
    // For now, we'll just assume current is within limits
}

/**
 * Trigger a failsafe action
 */
void failsafe_trigger(uint8_t reason) {
    // If failsafe is already active, don't trigger again
    if (failsafeState.isActive) {
        return;
    }
    
    ESP_LOGW(TAG, "Triggering failsafe: reason %d", reason);
    
    // Set failsafe state
    failsafeState.isActive = true;
    failsafeState.reason = reason;
    failsafeState.activationTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Update system state
    failsafeState.systemState->failsafeActive = true;
    failsafeState.systemState->failsafeReason = reason;
    
    // Take appropriate action based on reason
    switch (reason) {
        case FAILSAFE_TIMEOUT:
            // Stop motors
            motor_emergency_stop();
            // Flash lights to indicate timeout
            lights_set_indicators(INDICATOR_HAZARD);
            break;
            
        case FAILSAFE_BATTERY_CRITICAL:
            // Stop motors
            motor_emergency_stop();
            // Flash lights to indicate critical battery
            lights_flash_headlights(3, 200);
            lights_flash_taillights(3, 200);
            break;
            
        case FAILSAFE_OVERCURRENT:
            // Stop motors
            motor_emergency_stop();
            // Flash lights to indicate overcurrent
            lights_flash_headlights(2, 100);
            break;
            
        case FAILSAFE_MOTOR_BLOCKED:
            // Stop motors
            motor_emergency_stop();
            // Flash lights to indicate motor blocked
            lights_flash_taillights(2, 100);
            break;
            
        case FAILSAFE_SYSTEM_ERROR:
            // Stop motors
            motor_emergency_stop();
            // Flash lights to indicate system error
            lights_set_indicators(INDICATOR_HAZARD);
            lights_flash_headlights(5, 100);
            break;
            
        default:
            // Unknown reason, stop motors
            motor_emergency_stop();
            break;
    }
}

/**
 * Reset the failsafe system
 */
bool failsafe_reset(void) {
    // Check if we can reset based on the reason
    if (failsafeState.reason == FAILSAFE_BATTERY_CRITICAL) {
        // Can't reset if battery is still critical
        if (failsafeState.systemState->batteryCritical) {
            ESP_LOGW(TAG, "Cannot reset failsafe: battery still critical");
            return false;
        }
    }
    
    ESP_LOGI(TAG, "Resetting failsafe system");
    
    // Reset failsafe state
    failsafeState.isActive = false;
    failsafeState.reason = FAILSAFE_NONE;
    
    // Update system state
    failsafeState.systemState->failsafeActive = false;
    failsafeState.systemState->failsafeReason = FAILSAFE_NONE;
    
    // Reset indicators
    lights_set_indicators(INDICATOR_NONE);
    
    // Update last command time to prevent immediate timeout
    failsafeState.lastCommandTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    return true;
}

/**
 * Check if a failsafe condition is active
 */
bool failsafe_is_active(void) {
    return failsafeState.isActive;
}

/**
 * Get the reason for the current failsafe condition
 */
uint8_t failsafe_get_reason(void) {
    return failsafeState.reason;
}

/**
 * Update the last command time
 */
void failsafe_update_command_time(void) {
    failsafeState.lastCommandTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    failsafeState.systemState->lastCommandTime = failsafeState.lastCommandTime;
    failsafeState.systemState->connectionActive = true;
}