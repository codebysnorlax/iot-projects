/**
 * ESP32 Smart Car - Motor Control Header
 * 
 * This file contains the declarations for the motor control module.
 * It handles the tank-style movement of the smart car.
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "config.h"

/**
 * Initialize the motor control module
 * Sets up GPIO pins, PWM channels, and initializes motor state
 */
void motor_init(void);

/**
 * Update motor speeds based on current control inputs
 * This function should be called regularly from the motor control task
 * 
 * @param state Pointer to the system state structure
 */
void motor_update(SystemState_t *state);

/**
 * Set the target speed for the left motor
 * 
 * @param speed Target speed (-100 to 100, negative for reverse)
 */
void motor_set_left_speed(int8_t speed);

/**
 * Set the target speed for the right motor
 * 
 * @param speed Target speed (-100 to 100, negative for reverse)
 */
void motor_set_right_speed(int8_t speed);

/**
 * Set the target speeds for both motors
 * 
 * @param leftSpeed Target speed for left motor (-100 to 100)
 * @param rightSpeed Target speed for right motor (-100 to 100)
 */
void motor_set_speeds(int8_t leftSpeed, int8_t rightSpeed);

/**
 * Get the current speed of the left motor
 * 
 * @return Current speed (-100 to 100, negative for reverse)
 */
int8_t motor_get_left_speed(void);

/**
 * Get the current speed of the right motor
 * 
 * @return Current speed (-100 to 100, negative for reverse)
 */
int8_t motor_get_right_speed(void);

/**
 * Stop both motors (emergency stop)
 * This function immediately stops both motors without ramping down
 */
void motor_emergency_stop(void);

/**
 * Smoothly stop both motors
 * This function gradually reduces the speed of both motors to zero
 */
void motor_smooth_stop(void);

/**
 * Check if motors are currently stopped
 * 
 * @return true if both motors are stopped, false otherwise
 */
bool motor_is_stopped(void);

/**
 * Calibrate the motors
 * This function can be used to calibrate the minimum PWM values needed to move the motors
 * 
 * @return true if calibration was successful, false otherwise
 */
bool motor_calibrate(void);

#endif // MOTOR_H