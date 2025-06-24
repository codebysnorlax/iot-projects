/**
 * ESP32 Smart Car - Lighting System Implementation
 * 
 * This file implements the lighting system functionality for the ESP32 Smart Car.
 * It handles the headlights, taillights, indicators, and underglow LEDs.
 */

#include "lights.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "lights";

// PWM channel configuration
#define LIGHT_HEADLIGHT_PWM_CHANNEL LEDC_CHANNEL_2
#define LIGHT_UNDERGLOW_PWM_CHANNEL LEDC_CHANNEL_3
#define LIGHT_PWM_TIMER            LEDC_TIMER_1

// Lighting state
typedef struct {
    bool headlightsOn;
    uint8_t headlightsBrightness;
    bool taillightsOn;
    uint8_t indicatorState;
    bool indicatorOn;  // For blinking
    uint32_t indicatorLastToggle;
    uint8_t underglowR;
    uint8_t underglowG;
    uint8_t underglowB;
    uint8_t underglowMode;
    uint32_t underglowLastUpdate;
    uint8_t underglowAnimationStep;
} LightState_t;

static LightState_t lightState = {0};

// Private function prototypes
static void light_set_pwm(ledc_channel_t channel, uint32_t duty);
static void update_indicators(void);
static void update_underglow(void);

/**
 * Initialize the lighting system
 */
void lights_init(void) {
    ESP_LOGI(TAG, "Initializing lighting system");
    
    // Configure GPIO pins for digital outputs
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LIGHT_TAILLIGHT_PIN) | 
                          (1ULL << LIGHT_INDICATOR_L_PIN) | 
                          (1ULL << LIGHT_INDICATOR_R_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    // Configure PWM timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = (ledc_timer_bit_t)LIGHT_PWM_RESOLUTION,
        .timer_num = LIGHT_PWM_TIMER,
        .freq_hz = LIGHT_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    // Configure PWM channel for headlights
    ledc_channel_config_t ledc_channel_headlight = {
        .gpio_num = LIGHT_HEADLIGHT_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LIGHT_HEADLIGHT_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LIGHT_PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_headlight));
    
    // Configure PWM channel for underglow
    ledc_channel_config_t ledc_channel_underglow = {
        .gpio_num = LIGHT_UNDERGLOW_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LIGHT_UNDERGLOW_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LIGHT_PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_underglow));
    
    // Initialize light state
    lightState.headlightsOn = false;
    lightState.headlightsBrightness = LIGHT_HEADLIGHT_MAX;
    lightState.taillightsOn = false;
    lightState.indicatorState = INDICATOR_NONE;
    lightState.indicatorOn = false;
    lightState.indicatorLastToggle = 0;
    lightState.underglowR = 0;
    lightState.underglowG = 0;
    lightState.underglowB = 0;
    lightState.underglowMode = 0;
    lightState.underglowLastUpdate = 0;
    lightState.underglowAnimationStep = 0;
    
    // Set initial states
    gpio_set_level(LIGHT_TAILLIGHT_PIN, 0);
    gpio_set_level(LIGHT_INDICATOR_L_PIN, 0);
    gpio_set_level(LIGHT_INDICATOR_R_PIN, 0);
    light_set_pwm(LIGHT_HEADLIGHT_PWM_CHANNEL, 0);
    
    // For a real implementation, you would initialize the WS2812B underglow LEDs here
    // This would typically involve setting up an SPI or RMT peripheral
    // For simplicity, we're just using a PWM pin as a placeholder
    light_set_pwm(LIGHT_UNDERGLOW_PWM_CHANNEL, 0);
    
    ESP_LOGI(TAG, "Lighting system initialized");
}

/**
 * Update the lighting system
 */
void lights_update(void) {
    // Get current time
    uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Update indicators (blinking)
    update_indicators();
    
    // Update underglow animations
    update_underglow();
}

/**
 * Set the headlights state
 */
void lights_set_headlights(bool on) {
    lightState.headlightsOn = on;
    
    if (on) {
        light_set_pwm(LIGHT_HEADLIGHT_PWM_CHANNEL, lightState.headlightsBrightness);
        ESP_LOGI(TAG, "Headlights turned ON");
    } else {
        light_set_pwm(LIGHT_HEADLIGHT_PWM_CHANNEL, 0);
        ESP_LOGI(TAG, "Headlights turned OFF");
    }
}

/**
 * Set the headlights brightness
 */
void lights_set_headlights_brightness(uint8_t brightness) {
    lightState.headlightsBrightness = brightness;
    
    if (lightState.headlightsOn) {
        light_set_pwm(LIGHT_HEADLIGHT_PWM_CHANNEL, brightness);
        ESP_LOGI(TAG, "Headlights brightness set to %d", brightness);
    }
}

/**
 * Set the taillights state
 */
void lights_set_taillights(bool on) {
    lightState.taillightsOn = on;
    gpio_set_level(LIGHT_TAILLIGHT_PIN, on ? 1 : 0);
    ESP_LOGI(TAG, "Taillights turned %s", on ? "ON" : "OFF");
}

/**
 * Set the indicators state
 */
void lights_set_indicators(uint8_t state) {
    if (state > INDICATOR_HAZARD) {
        ESP_LOGW(TAG, "Invalid indicator state: %d", state);
        return;
    }
    
    lightState.indicatorState = state;
    
    // Reset blink state
    lightState.indicatorOn = false;
    lightState.indicatorLastToggle = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Turn off indicators initially
    gpio_set_level(LIGHT_INDICATOR_L_PIN, 0);
    gpio_set_level(LIGHT_INDICATOR_R_PIN, 0);
    
    ESP_LOGI(TAG, "Indicators set to state %d", state);
}

/**
 * Set the underglow color
 */
void lights_set_underglow(uint8_t r, uint8_t g, uint8_t b) {
    lightState.underglowR = r;
    lightState.underglowG = g;
    lightState.underglowB = b;
    lightState.underglowMode = 0; // Static color mode
    
    // For a real implementation, you would update the WS2812B LEDs here
    // For now, we'll just set the brightness based on the average RGB value
    uint8_t brightness = (r + g + b) / 3;
    light_set_pwm(LIGHT_UNDERGLOW_PWM_CHANNEL, brightness);
    
    ESP_LOGI(TAG, "Underglow set to RGB(%d,%d,%d)", r, g, b);
}

/**
 * Set the underglow animation mode
 */
void lights_set_underglow_mode(uint8_t mode) {
    lightState.underglowMode = mode;
    lightState.underglowAnimationStep = 0;
    lightState.underglowLastUpdate = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    ESP_LOGI(TAG, "Underglow mode set to %d", mode);
}

/**
 * Get the current headlights state
 */
bool lights_get_headlights_state(void) {
    return lightState.headlightsOn;
}

/**
 * Get the current taillights state
 */
bool lights_get_taillights_state(void) {
    return lightState.taillightsOn;
}

/**
 * Get the current indicators state
 */
uint8_t lights_get_indicators_state(void) {
    return lightState.indicatorState;
}

/**
 * Get the current underglow color
 */
void lights_get_underglow_color(uint8_t *r, uint8_t *g, uint8_t *b) {
    if (r) *r = lightState.underglowR;
    if (g) *g = lightState.underglowG;
    if (b) *b = lightState.underglowB;
}

/**
 * Get the current underglow animation mode
 */
uint8_t lights_get_underglow_mode(void) {
    return lightState.underglowMode;
}

/**
 * Flash the headlights
 */
void lights_flash_headlights(uint8_t count, uint16_t duration) {
    // Save current state
    bool previousState = lightState.headlightsOn;
    
    for (uint8_t i = 0; i < count; i++) {
        // Turn on
        lights_set_headlights(true);
        vTaskDelay(pdMS_TO_TICKS(duration));
        
        // Turn off
        lights_set_headlights(false);
        if (i < count - 1) {
            vTaskDelay(pdMS_TO_TICKS(duration));
        }
    }
    
    // Restore previous state
    lights_set_headlights(previousState);
}

/**
 * Flash the taillights
 */
void lights_flash_taillights(uint8_t count, uint16_t duration) {
    // Save current state
    bool previousState = lightState.taillightsOn;
    
    for (uint8_t i = 0; i < count; i++) {
        // Turn on
        lights_set_taillights(true);
        vTaskDelay(pdMS_TO_TICKS(duration));
        
        // Turn off
        lights_set_taillights(false);
        if (i < count - 1) {
            vTaskDelay(pdMS_TO_TICKS(duration));
        }
    }
    
    // Restore previous state
    lights_set_taillights(previousState);
}

/**
 * Set PWM duty cycle for a light channel
 */
static void light_set_pwm(ledc_channel_t channel, uint32_t duty) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel));
}

/**
 * Update indicators (handle blinking)
 */
static void update_indicators(void) {
    // If indicators are off, nothing to do
    if (lightState.indicatorState == INDICATOR_NONE) {
        return;
    }
    
    uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Check if it's time to toggle the indicators
    if (currentTime - lightState.indicatorLastToggle >= LIGHT_INDICATOR_PERIOD) {
        // Toggle indicator state
        lightState.indicatorOn = !lightState.indicatorOn;
        lightState.indicatorLastToggle = currentTime;
        
        // Update indicator lights based on state
        switch (lightState.indicatorState) {
            case INDICATOR_LEFT:
                gpio_set_level(LIGHT_INDICATOR_L_PIN, lightState.indicatorOn ? 1 : 0);
                gpio_set_level(LIGHT_INDICATOR_R_PIN, 0);
                break;
                
            case INDICATOR_RIGHT:
                gpio_set_level(LIGHT_INDICATOR_L_PIN, 0);
                gpio_set_level(LIGHT_INDICATOR_R_PIN, lightState.indicatorOn ? 1 : 0);
                break;
                
            case INDICATOR_HAZARD:
                gpio_set_level(LIGHT_INDICATOR_L_PIN, lightState.indicatorOn ? 1 : 0);
                gpio_set_level(LIGHT_INDICATOR_R_PIN, lightState.indicatorOn ? 1 : 0);
                break;
        }
    }
}

/**
 * Update underglow animations
 */
static void update_underglow(void) {
    uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Only update animations every 50ms
    if (currentTime - lightState.underglowLastUpdate < 50) {
        return;
    }
    
    lightState.underglowLastUpdate = currentTime;
    
    // Handle different animation modes
    switch (lightState.underglowMode) {
        case 0: // Static color - nothing to update
            break;
            
        case 1: // Breathing effect
            {
                // Simple breathing effect using sine wave
                lightState.underglowAnimationStep = (lightState.underglowAnimationStep + 1) % 256;
                float factor = (sinf(lightState.underglowAnimationStep * 3.14159f / 128.0f) + 1.0f) / 2.0f;
                
                uint8_t r = lightState.underglowR * factor;
                uint8_t g = lightState.underglowG * factor;
                uint8_t b = lightState.underglowB * factor;
                
                // For a real implementation, you would update the WS2812B LEDs here
                // For now, we'll just set the brightness based on the average RGB value
                uint8_t brightness = (r + g + b) / 3;
                light_set_pwm(LIGHT_UNDERGLOW_PWM_CHANNEL, brightness);
            }
            break;
            
        case 2: // Rainbow effect
            {
                // Simple rainbow effect
                lightState.underglowAnimationStep = (lightState.underglowAnimationStep + 1) % 256;
                
                // HSV to RGB conversion (simplified)
                uint8_t region = lightState.underglowAnimationStep / 43;
                uint8_t remainder = (lightState.underglowAnimationStep - (region * 43)) * 6;
                
                uint8_t p = 0;
                uint8_t q = 0;
                uint8_t t = 0;
                
                uint8_t r, g, b;
                
                switch (region) {
                    case 0: r = 255; g = remainder; b = 0; break;
                    case 1: r = 255 - remainder; g = 255; b = 0; break;
                    case 2: r = 0; g = 255; b = remainder; break;
                    case 3: r = 0; g = 255 - remainder; b = 255; break;
                    case 4: r = remainder; g = 0; b = 255; break;
                    default: r = 255; g = 0; b = 255 - remainder; break;
                }
                
                // For a real implementation, you would update the WS2812B LEDs here
                // For now, we'll just set the brightness based on the average RGB value
                uint8_t brightness = (r + g + b) / 3;
                light_set_pwm(LIGHT_UNDERGLOW_PWM_CHANNEL, brightness);
            }
            break;
            
        case 3: // Chase effect
            {
                // Simple chase effect
                lightState.underglowAnimationStep = (lightState.underglowAnimationStep + 1) % LIGHT_UNDERGLOW_COUNT;
                
                // For a real implementation, you would update the WS2812B LEDs here
                // For now, we'll just toggle the PWM output
                light_set_pwm(LIGHT_UNDERGLOW_PWM_CHANNEL, 
                             lightState.underglowAnimationStep == 0 ? 255 : 0);
            }
            break;
    }
}