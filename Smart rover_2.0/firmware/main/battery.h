/**
 * ESP32 Smart Car - Battery Monitoring Header
 * 
 * This file contains the declarations for the battery monitoring module.
 * It handles battery voltage measurement and state of charge estimation.
 */

#ifndef BATTERY_H
#define BATTERY_H

#include "config.h"

/**
 * Initialize the battery monitoring module
 * Sets up ADC for battery voltage measurement
 */
void battery_init(void);

/**
 * Read the current battery voltage
 * 
 * @return Battery voltage in volts
 */
float battery_read_voltage(void);

/**
 * Calculate battery percentage based on voltage
 * 
 * @param voltage Battery voltage in volts
 * @return Battery percentage (0-100)
 */
int battery_calculate_percentage(float voltage);

/**
 * Check if battery is low
 * 
 * @return true if battery is low, false otherwise
 */
bool battery_is_low(void);

/**
 * Check if battery is critically low
 * 
 * @return true if battery is critically low, false otherwise
 */
bool battery_is_critical(void);

/**
 * Calibrate the battery voltage measurement
 * This function can be used to calibrate the voltage divider ratio
 * 
 * @param measured_voltage The actual battery voltage measured with a multimeter
 * @return true if calibration was successful, false otherwise
 */
bool battery_calibrate(float measured_voltage);

/**
 * Get the battery voltage divider ratio
 * 
 * @return The voltage divider ratio (Vbat / Vadc)
 */
float battery_get_divider_ratio(void);

/**
 * Set the battery voltage divider ratio
 * 
 * @param ratio The voltage divider ratio (Vbat / Vadc)
 */
void battery_set_divider_ratio(float ratio);

#endif // BATTERY_H