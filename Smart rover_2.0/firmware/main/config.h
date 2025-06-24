/**
 * ESP32 Smart Car - Configuration
 * 
 * This file contains all the configurable parameters for the ESP32 Smart Car project.
 * Modify these values to customize the behavior of your smart car.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/**
 * System Configuration
 */
#define FIRMWARE_VERSION        "1.0.0"
#define SYSTEM_NAME             "ESP32 Smart Car"

/**
 * Wi-Fi Configuration
 */
#define WIFI_AP_SSID           "ESP32-SmartCar"
#define WIFI_AP_PASSWORD       "smartcar123"
#define WIFI_AP_CHANNEL        1
#define WIFI_AP_MAX_CONN       4

/**
 * Web Server Configuration
 */
#define WEB_SERVER_PORT        80
#define WEBSOCKET_PATH         "/ws"
#define MAX_WEBSOCKET_CLIENTS  4

/**
 * Task Intervals
 */
#define MOTOR_UPDATE_INTERVAL_MS      20    // 50Hz motor control update
#define BATTERY_CHECK_INTERVAL_MS     1000  // Check battery every second
#define FAILSAFE_CHECK_INTERVAL_MS    100   // Check failsafe conditions every 100ms
#define LIGHT_UPDATE_INTERVAL_MS      50    // Update lights every 50ms (for animations)
#define WEBSOCKET_UPDATE_INTERVAL_MS  100   // Send updates to clients every 100ms

/**
 * Motor Configuration
 */
// Motor pins
#define MOTOR_LEFT_ENA_PIN     25  // PWM pin for left motor speed control
#define MOTOR_LEFT_IN1_PIN     26  // Left motor direction control 1
#define MOTOR_LEFT_IN2_PIN     27  // Left motor direction control 2
#define MOTOR_RIGHT_ENB_PIN    14  // PWM pin for right motor speed control
#define MOTOR_RIGHT_IN3_PIN    12  // Right motor direction control 1
#define MOTOR_RIGHT_IN4_PIN    13  // Right motor direction control 2

// Motor PWM configuration
#define MOTOR_PWM_FREQ         5000      // 5kHz PWM frequency
#define MOTOR_PWM_RESOLUTION   8         // 8-bit resolution (0-255)
#define MOTOR_PWM_MIN_DUTY     50        // Minimum duty cycle to start moving
#define MOTOR_PWM_MAX_DUTY     255       // Maximum duty cycle

// Motor control parameters
#define MOTOR_ACCELERATION     10        // Speed change per update cycle
#define MOTOR_DECELERATION     20        // Speed change per update cycle when stopping
#define MOTOR_DIRECTION_DELAY  100       // Delay in ms when changing direction

/**
 * Lighting System Configuration
 */
// Light pins
#define LIGHT_HEADLIGHT_PIN    4   // PWM pin for headlights
#define LIGHT_TAILLIGHT_PIN    16  // Pin for taillights
#define LIGHT_INDICATOR_L_PIN  17  // Pin for left indicator
#define LIGHT_INDICATOR_R_PIN  18  // Pin for right indicator
#define LIGHT_UNDERGLOW_PIN    19  // PWM pin for underglow (WS2812B)

// Light parameters
#define LIGHT_PWM_FREQ         5000      // 5kHz PWM frequency
#define LIGHT_PWM_RESOLUTION   8         // 8-bit resolution (0-255)
#define LIGHT_HEADLIGHT_MAX    255       // Maximum headlight brightness
#define LIGHT_INDICATOR_PERIOD 500       // Indicator blink period in ms
#define LIGHT_UNDERGLOW_COUNT  8         // Number of underglow LEDs

/**
 * Battery Monitoring Configuration
 */
#define BATTERY_ADC_PIN        34        // ADC pin for battery voltage measurement
#define BATTERY_ADC_SAMPLES    10        // Number of ADC samples to average
#define BATTERY_R1             10000     // Voltage divider R1 value (ohms)
#define BATTERY_R2             3300      // Voltage divider R2 value (ohms)
#define BATTERY_ADC_MAX        4095      // Maximum ADC value (12-bit)
#define BATTERY_ADC_VREF       1100      // ADC reference voltage in mV

// Battery thresholds
#define BATTERY_FULL_VOLTAGE   12.6      // Fully charged battery voltage (3S Li-ion)
#define BATTERY_EMPTY_VOLTAGE  9.0       // Empty battery voltage (3S Li-ion)
#define BATTERY_LOW_THRESHOLD  10.0      // Low battery warning threshold
#define BATTERY_CRITICAL_THRESHOLD 9.5   // Critical battery threshold

/**
 * Horn Configuration
 */
#define HORN_PIN               5         // Pin for horn/buzzer

/**
 * Failsafe Configuration
 */
#define FAILSAFE_TIMEOUT_MS    1000      // Communication timeout in ms
#define FAILSAFE_MAX_CURRENT   5000      // Maximum current in mA (if current sensing is implemented)
#define FAILSAFE_RECOVERY_TIME 3000      // Time in ms before recovery from failsafe

/**
 * System State Structure
 * This structure holds the current state of the system and is shared between tasks
 */
typedef struct {
    // System status
    bool isInitialized;
    uint32_t uptime;          // Uptime in seconds
    
    // Control inputs
    int8_t leftMotorCommand;   // -100 to 100
    int8_t rightMotorCommand;  // -100 to 100
    
    // Motor status
    int8_t leftMotorSpeed;     // -100 to 100
    int8_t rightMotorSpeed;    // -100 to 100
    
    // Light status
    bool headlightsOn;
    bool taillightsOn;
    uint8_t indicatorState;    // 0=off, 1=left, 2=right, 3=hazard
    uint8_t underglowR;
    uint8_t underglowG;
    uint8_t underglowB;
    
    // Horn status
    bool hornOn;
    
    // Battery status
    float batteryVoltage;
    int batteryPercentage;
    bool batteryLow;
    bool batteryCritical;
    
    // Failsafe status
    bool failsafeActive;
    uint8_t failsafeReason;
    
    // Communication status
    uint32_t lastCommandTime;  // Time of last command in ms
    bool connectionActive;
} SystemState_t;

/**
 * Indicator States
 */
enum {
    INDICATOR_NONE = 0,
    INDICATOR_LEFT = 1,
    INDICATOR_RIGHT = 2,
    INDICATOR_HAZARD = 3
};

/**
 * Failsafe Reasons
 */
enum {
    FAILSAFE_NONE = 0,
    FAILSAFE_TIMEOUT = 1,
    FAILSAFE_BATTERY_CRITICAL = 2,
    FAILSAFE_OVERCURRENT = 3,
    FAILSAFE_MOTOR_BLOCKED = 4,
    FAILSAFE_SYSTEM_ERROR = 5
};

#endif // CONFIG_H