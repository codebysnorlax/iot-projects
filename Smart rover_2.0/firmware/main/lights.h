/**
 * ESP32 Smart Car - Lighting System Header
 * 
 * This file contains the declarations for the lighting system module.
 * It handles the headlights, taillights, indicators, and underglow LEDs.
 */

#ifndef LIGHTS_H
#define LIGHTS_H

#include "config.h"

/**
 * Initialize the lighting system
 * Sets up GPIO pins, PWM channels, and initializes light state
 */
void lights_init(void);

/**
 * Update the lighting system
 * This function should be called regularly to handle blinking indicators and animations
 */
void lights_update(void);

/**
 * Set the headlights state
 * 
 * @param on true to turn on headlights, false to turn off
 */
void lights_set_headlights(bool on);

/**
 * Set the headlights brightness
 * 
 * @param brightness Brightness level (0-255)
 */
void lights_set_headlights_brightness(uint8_t brightness);

/**
 * Set the taillights state
 * 
 * @param on true to turn on taillights, false to turn off
 */
void lights_set_taillights(bool on);

/**
 * Set the indicators state
 * 
 * @param state Indicator state (INDICATOR_NONE, INDICATOR_LEFT, INDICATOR_RIGHT, INDICATOR_HAZARD)
 */
void lights_set_indicators(uint8_t state);

/**
 * Set the underglow color
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void lights_set_underglow(uint8_t r, uint8_t g, uint8_t b);

/**
 * Set the underglow animation mode
 * 
 * @param mode Animation mode (0=static, 1=breathing, 2=rainbow, 3=chase)
 */
void lights_set_underglow_mode(uint8_t mode);

/**
 * Get the current headlights state
 * 
 * @return true if headlights are on, false otherwise
 */
bool lights_get_headlights_state(void);

/**
 * Get the current taillights state
 * 
 * @return true if taillights are on, false otherwise
 */
bool lights_get_taillights_state(void);

/**
 * Get the current indicators state
 * 
 * @return Indicator state (INDICATOR_NONE, INDICATOR_LEFT, INDICATOR_RIGHT, INDICATOR_HAZARD)
 */
uint8_t lights_get_indicators_state(void);

/**
 * Get the current underglow color
 * 
 * @param r Pointer to store red component
 * @param g Pointer to store green component
 * @param b Pointer to store blue component
 */
void lights_get_underglow_color(uint8_t *r, uint8_t *g, uint8_t *b);

/**
 * Get the current underglow animation mode
 * 
 * @return Animation mode (0=static, 1=breathing, 2=rainbow, 3=chase)
 */
uint8_t lights_get_underglow_mode(void);

/**
 * Flash the headlights
 * 
 * @param count Number of flashes
 * @param duration Duration of each flash in milliseconds
 */
void lights_flash_headlights(uint8_t count, uint16_t duration);

/**
 * Flash the taillights
 * 
 * @param count Number of flashes
 * @param duration Duration of each flash in milliseconds
 */
void lights_flash_taillights(uint8_t count, uint16_t duration);

#endif // LIGHTS_H