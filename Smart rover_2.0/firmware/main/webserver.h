/**
 * ESP32 Smart Car - Web Server Header
 * 
 * This file contains the declarations for the web server module.
 * It handles the web interface and WebSocket communication for real-time control.
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "config.h"
#include "esp_http_server.h"

/**
 * Initialize the web server
 * Sets up the HTTP server and WebSocket handler
 * 
 * @param state Pointer to the system state structure
 */
void webserver_init(SystemState_t *state);

/**
 * Start the web server
 * This function blocks and should be called from its own task
 */
void webserver_start(void);

/**
 * Stop the web server
 */
void webserver_stop(void);

/**
 * Send system state update to all connected WebSocket clients
 * This function should be called regularly to update the web interface
 */
void webserver_send_state_update(void);

/**
 * Check if any clients are connected
 * 
 * @return true if at least one client is connected, false otherwise
 */
bool webserver_has_clients(void);

/**
 * Get the number of connected clients
 * 
 * @return Number of connected clients
 */
uint8_t webserver_get_client_count(void);

/**
 * Register OTA update handlers
 * This function sets up the HTTP handlers for OTA updates
 * 
 * @param server HTTP server handle
 */
esp_err_t webserver_register_ota_handlers(httpd_handle_t server);

#endif // WEBSERVER_H