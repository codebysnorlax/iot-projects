/**
 * ESP32 Smart Car - Motor Control Implementation
 * 
 * This file implements the motor control functionality for the ESP32 Smart Car.
 * It handles the tank-style movement with independent control of left and right motors.
 */

#include "motor.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "motor";

// PWM channel configuration
#define MOTOR_LEFT_PWM_CHANNEL  LEDC_CHANNEL_0
#define MOTOR_RIGHT_PWM_CHANNEL LEDC_CHANNEL_1
#define MOTOR_PWM_TIMER         LEDC_TIMER_0

// Motor state variables
typedef struct {
    int8_t targetSpeed;    // Target speed (-100 to 100)
    int8_t currentSpeed;   // Current speed (-100 to 100)
    uint8_t pwmDuty;       // Current PWM duty cycle
    bool isForward;        // Current direction
    bool isChangingDir;    // Flag for direction change in progress
    uint32_t dirChangeTime; // Time when direction change started
} MotorState_t;

static MotorState_t leftMotor = {0};
static MotorState_t rightMotor = {0};

// Private function prototypes
static void motor_set_pwm(ledc_channel_t channel, uint32_t duty);
static void motor_set_direction(MotorState_t *motor, gpio_num_t pin1, gpio_num_t pin2, bool forward);
static uint8_t speed_to_duty(int8_t speed);

/**
 * Initialize the motor control module
 */
void motor_init(void) {
    ESP_LOGI(TAG, "Initializing motor control");
    
    // Configure GPIO pins
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << MOTOR_LEFT_IN1_PIN) | 
                          (1ULL << MOTOR_LEFT_IN2_PIN) | 
                          (1ULL << MOTOR_RIGHT_IN3_PIN) | 
                          (1ULL << MOTOR_RIGHT_IN4_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    // Configure PWM timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = (ledc_timer_bit_t)MOTOR_PWM_RESOLUTION,
        .timer_num = MOTOR_PWM_TIMER,
        .freq_hz = MOTOR_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    // Configure PWM channels for left motor
    ledc_channel_config_t ledc_channel_left = {
        .gpio_num = MOTOR_LEFT_ENA_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = MOTOR_LEFT_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = MOTOR_PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_left));
    
    // Configure PWM channels for right motor
    ledc_channel_config_t ledc_channel_right = {
        .gpio_num = MOTOR_RIGHT_ENB_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = MOTOR_RIGHT_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = MOTOR_PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_right));
    
    // Initialize motor state
    leftMotor.targetSpeed = 0;
    leftMotor.currentSpeed = 0;
    leftMotor.pwmDuty = 0;
    leftMotor.isForward = true;
    leftMotor.isChangingDir = false;
    
    rightMotor.targetSpeed = 0;
    rightMotor.currentSpeed = 0;
    rightMotor.pwmDuty = 0;
    rightMotor.isForward = true;
    rightMotor.isChangingDir = false;
    
    // Set initial motor directions
    motor_set_direction(&leftMotor, MOTOR_LEFT_IN1_PIN, MOTOR_LEFT_IN2_PIN, true);
    motor_set_direction(&rightMotor, MOTOR_RIGHT_IN3_PIN, MOTOR_RIGHT_IN4_PIN, true);
    
    // Set initial PWM duty cycles to 0
    motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, 0);
    motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, 0);
    
    ESP_LOGI(TAG, "Motor control initialized");
}

/**
 * Update motor speeds based on current control inputs
 */
void motor_update(SystemState_t *state) {
    // Get target speeds from system state
    int8_t leftTarget = state->leftMotorCommand;
    int8_t rightTarget = state->rightMotorCommand;
    
    // Set target speeds
    motor_set_left_speed(leftTarget);
    motor_set_right_speed(rightTarget);
    
    // Update left motor
    if (leftMotor.isChangingDir) {
        // Check if direction change delay has passed
        if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - leftMotor.dirChangeTime >= MOTOR_DIRECTION_DELAY) {
            leftMotor.isChangingDir = false;
            // Set new direction
            bool newDirection = (leftMotor.targetSpeed >= 0);
            motor_set_direction(&leftMotor, MOTOR_LEFT_IN1_PIN, MOTOR_LEFT_IN2_PIN, newDirection);
        }
    } else {
        // Check if direction change is needed
        if ((leftMotor.currentSpeed > 0 && leftMotor.targetSpeed < 0) || 
            (leftMotor.currentSpeed < 0 && leftMotor.targetSpeed > 0)) {
            // Need to change direction, first stop the motor
            leftMotor.currentSpeed = 0;
            leftMotor.pwmDuty = 0;
            motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, 0);
            leftMotor.isChangingDir = true;
            leftMotor.dirChangeTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        } else {
            // Normal speed adjustment
            if (abs(leftMotor.currentSpeed) < abs(leftMotor.targetSpeed)) {
                // Accelerate
                if (leftMotor.targetSpeed > 0) {
                    leftMotor.currentSpeed = MIN(leftMotor.currentSpeed + MOTOR_ACCELERATION, leftMotor.targetSpeed);
                } else {
                    leftMotor.currentSpeed = MAX(leftMotor.currentSpeed - MOTOR_ACCELERATION, leftMotor.targetSpeed);
                }
            } else if (abs(leftMotor.currentSpeed) > abs(leftMotor.targetSpeed)) {
                // Decelerate
                if (leftMotor.currentSpeed > 0) {
                    leftMotor.currentSpeed = MAX(leftMotor.currentSpeed - MOTOR_DECELERATION, leftMotor.targetSpeed);
                } else {
                    leftMotor.currentSpeed = MIN(leftMotor.currentSpeed + MOTOR_DECELERATION, leftMotor.targetSpeed);
                }
            }
            
            // Update PWM duty cycle
            leftMotor.pwmDuty = speed_to_duty(abs(leftMotor.currentSpeed));
            motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, leftMotor.pwmDuty);
        }
    }
    
    // Update right motor (same logic as left motor)
    if (rightMotor.isChangingDir) {
        // Check if direction change delay has passed
        if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - rightMotor.dirChangeTime >= MOTOR_DIRECTION_DELAY) {
            rightMotor.isChangingDir = false;
            // Set new direction
            bool newDirection = (rightMotor.targetSpeed >= 0);
            motor_set_direction(&rightMotor, MOTOR_RIGHT_IN3_PIN, MOTOR_RIGHT_IN4_PIN, newDirection);
        }
    } else {
        // Check if direction change is needed
        if ((rightMotor.currentSpeed > 0 && rightMotor.targetSpeed < 0) || 
            (rightMotor.currentSpeed < 0 && rightMotor.targetSpeed > 0)) {
            // Need to change direction, first stop the motor
            rightMotor.currentSpeed = 0;
            rightMotor.pwmDuty = 0;
            motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, 0);
            rightMotor.isChangingDir = true;
            rightMotor.dirChangeTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        } else {
            // Normal speed adjustment
            if (abs(rightMotor.currentSpeed) < abs(rightMotor.targetSpeed)) {
                // Accelerate
                if (rightMotor.targetSpeed > 0) {
                    rightMotor.currentSpeed = MIN(rightMotor.currentSpeed + MOTOR_ACCELERATION, rightMotor.targetSpeed);
                } else {
                    rightMotor.currentSpeed = MAX(rightMotor.currentSpeed - MOTOR_ACCELERATION, rightMotor.targetSpeed);
                }
            } else if (abs(rightMotor.currentSpeed) > abs(rightMotor.targetSpeed)) {
                // Decelerate
                if (rightMotor.currentSpeed > 0) {
                    rightMotor.currentSpeed = MAX(rightMotor.currentSpeed - MOTOR_DECELERATION, rightMotor.targetSpeed);
                } else {
                    rightMotor.currentSpeed = MIN(rightMotor.currentSpeed + MOTOR_DECELERATION, rightMotor.targetSpeed);
                }
            }
            
            // Update PWM duty cycle
            rightMotor.pwmDuty = speed_to_duty(abs(rightMotor.currentSpeed));
            motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, rightMotor.pwmDuty);
        }
    }
}

/**
 * Set the target speed for the left motor
 */
void motor_set_left_speed(int8_t speed) {
    // Clamp speed to valid range
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;
    
    leftMotor.targetSpeed = speed;
}

/**
 * Set the target speed for the right motor
 */
void motor_set_right_speed(int8_t speed) {
    // Clamp speed to valid range
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;
    
    rightMotor.targetSpeed = speed;
}

/**
 * Set the target speeds for both motors
 */
void motor_set_speeds(int8_t leftSpeed, int8_t rightSpeed) {
    motor_set_left_speed(leftSpeed);
    motor_set_right_speed(rightSpeed);
}

/**
 * Get the current speed of the left motor
 */
int8_t motor_get_left_speed(void) {
    return leftMotor.currentSpeed;
}

/**
 * Get the current speed of the right motor
 */
int8_t motor_get_right_speed(void) {
    return rightMotor.currentSpeed;
}

/**
 * Stop both motors (emergency stop)
 */
void motor_emergency_stop(void) {
    ESP_LOGW(TAG, "Emergency stop triggered");
    
    // Set target and current speeds to 0
    leftMotor.targetSpeed = 0;
    leftMotor.currentSpeed = 0;
    rightMotor.targetSpeed = 0;
    rightMotor.currentSpeed = 0;
    
    // Set PWM duty cycles to 0
    leftMotor.pwmDuty = 0;
    rightMotor.pwmDuty = 0;
    motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, 0);
    motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, 0);
    
    // Reset direction change flags
    leftMotor.isChangingDir = false;
    rightMotor.isChangingDir = false;
}

/**
 * Smoothly stop both motors
 */
void motor_smooth_stop(void) {
    ESP_LOGI(TAG, "Smooth stop initiated");
    
    // Set target speeds to 0 but let the update function handle deceleration
    leftMotor.targetSpeed = 0;
    rightMotor.targetSpeed = 0;
}

/**
 * Check if motors are currently stopped
 */
bool motor_is_stopped(void) {
    return (leftMotor.currentSpeed == 0 && rightMotor.currentSpeed == 0 && 
            !leftMotor.isChangingDir && !rightMotor.isChangingDir);
}

/**
 * Calibrate the motors
 */
bool motor_calibrate(void) {
    ESP_LOGI(TAG, "Starting motor calibration");
    
    // This is a simplified calibration routine
    // In a real implementation, you would determine the minimum PWM values
    // needed to start the motors moving and store them in NVS
    
    // For now, we'll just test the motors at low speed
    
    // Test left motor forward
    ESP_LOGI(TAG, "Testing left motor forward");
    motor_set_direction(&leftMotor, MOTOR_LEFT_IN1_PIN, MOTOR_LEFT_IN2_PIN, true);
    motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, MOTOR_PWM_MIN_DUTY);
    vTaskDelay(pdMS_TO_TICKS(1000));
    motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test left motor backward
    ESP_LOGI(TAG, "Testing left motor backward");
    motor_set_direction(&leftMotor, MOTOR_LEFT_IN1_PIN, MOTOR_LEFT_IN2_PIN, false);
    motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, MOTOR_PWM_MIN_DUTY);
    vTaskDelay(pdMS_TO_TICKS(1000));
    motor_set_pwm(MOTOR_LEFT_PWM_CHANNEL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test right motor forward
    ESP_LOGI(TAG, "Testing right motor forward");
    motor_set_direction(&rightMotor, MOTOR_RIGHT_IN3_PIN, MOTOR_RIGHT_IN4_PIN, true);
    motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, MOTOR_PWM_MIN_DUTY);
    vTaskDelay(pdMS_TO_TICKS(1000));
    motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test right motor backward
    ESP_LOGI(TAG, "Testing right motor backward");
    motor_set_direction(&rightMotor, MOTOR_RIGHT_IN3_PIN, MOTOR_RIGHT_IN4_PIN, false);
    motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, MOTOR_PWM_MIN_DUTY);
    vTaskDelay(pdMS_TO_TICKS(1000));
    motor_set_pwm(MOTOR_RIGHT_PWM_CHANNEL, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Reset motor directions to forward
    motor_set_direction(&leftMotor, MOTOR_LEFT_IN1_PIN, MOTOR_LEFT_IN2_PIN, true);
    motor_set_direction(&rightMotor, MOTOR_RIGHT_IN3_PIN, MOTOR_RIGHT_IN4_PIN, true);
    
    ESP_LOGI(TAG, "Motor calibration complete");
    return true;
}

/**
 * Set PWM duty cycle for a motor channel
 */
static void motor_set_pwm(ledc_channel_t channel, uint32_t duty) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel));
}

/**
 * Set motor direction
 */
static void motor_set_direction(MotorState_t *motor, gpio_num_t pin1, gpio_num_t pin2, bool forward) {
    if (forward) {
        gpio_set_level(pin1, 1);
        gpio_set_level(pin2, 0);
    } else {
        gpio_set_level(pin1, 0);
        gpio_set_level(pin2, 1);
    }
    motor->isForward = forward;
}

/**
 * Convert speed percentage to PWM duty cycle
 */
static uint8_t speed_to_duty(int8_t speed) {
    if (speed <= 0) return 0;
    
    // Map speed (1-100) to duty cycle (MIN_DUTY-MAX_DUTY)
    return MOTOR_PWM_MIN_DUTY + ((MOTOR_PWM_MAX_DUTY - MOTOR_PWM_MIN_DUTY) * speed) / 100;
}