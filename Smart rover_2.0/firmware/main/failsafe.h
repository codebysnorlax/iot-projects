/**
 * ESP32 Smart Car - Failsafe System Header
 * 
 * This file contains the declarations for the failsafe system module.
 * It handles safety mechanisms like timeout detection, overcurrent protection, and motor blocking.
 */

#ifndef FAILSAFE_H
#define FAILSAFE_H

#include "config.h"

/**
 * Initialize the failsafe system
 * 
 * @param state Pointer to the system state structure
 */
void failsafe_init(SystemState_t *state);

/**
 * Check for failsafe conditions
 * This function should be called regularly from the failsafe task
 */
void failsafe_check(void);

/**
 * Trigger a failsafe action
 * 
 * @param reason The reason for triggering the failsafe
 */
void failsafe_trigger(uint8_t reason);

/**
 * Reset the failsafe system
 * This function attempts to recover from a failsafe condition
 * 
 * @return true if reset was successful, false otherwise
 */
bool failsafe_reset(void);

/**
 * Check if a failsafe condition is active
 * 
 * @return true if failsafe is active, false otherwise
 */
bool failsafe_is_active(void);

/**
 * Get the reason for the current failsafe condition
 * 
 * @return Failsafe reason code (see config.h for definitions)
 */
uint8_t failsafe_get_reason(void);

/**
 * Update the last command time
 * This function should be called whenever a valid command is received
 */
void failsafe_update_command_time(void);

#endif // FAILSAFE_H