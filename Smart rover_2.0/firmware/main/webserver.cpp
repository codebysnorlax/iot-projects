/**
 * ESP32 Smart Car - Web Server Implementation
 * 
 * This file implements the web server functionality for the ESP32 Smart Car.
 * It handles the web interface and WebSocket communication for real-time control.
 */

#include "webserver.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include "motor.h"
#include "lights.h"
#include "battery.h"
#include "failsafe.h"
#include <string.h>

static const char *TAG = "webserver";

// Web server handle
static httpd_handle_t server = NULL;

// System state pointer
static SystemState_t *systemState = NULL;

// WebSocket clients
typedef struct {
    bool inUse;
    int fd;
    uint32_t lastActivity;
} WebSocketClient_t;

static WebSocketClient_t wsClients[MAX_WEBSOCKET_CLIENTS] = {0};

// Task handle for state update task
static TaskHandle_t stateUpdateTaskHandle = NULL;

// Forward declarations for HTTP handlers
static esp_err_t index_handler(httpd_req_t *req);
static esp_err_t js_handler(httpd_req_t *req);
static esp_err_t css_handler(httpd_req_t *req);
static esp_err_t websocket_handler(httpd_req_t *req);
static esp_err_t calibration_handler(httpd_req_t *req);
static esp_err_t api_handler(httpd_req_t *req);

// Forward declarations for helper functions
static void state_update_task(void *pvParameters);
static void process_websocket_message(int client_fd, const char *msg, size_t len);
static void send_websocket_message(int client_fd, const char *msg);
static void broadcast_websocket_message(const char *msg);
static void generate_state_json(char *buffer, size_t buffer_size);

/**
 * Initialize the web server
 */
void webserver_init(SystemState_t *state) {
    ESP_LOGI(TAG, "Initializing web server");
    
    // Store pointer to system state
    systemState = state;
    
    // Initialize WebSocket client array
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
        wsClients[i].inUse = false;
        wsClients[i].fd = -1;
        wsClients[i].lastActivity = 0;
    }
    
    // Create state update task
    xTaskCreatePinnedToCore(
        state_update_task,
        "StateUpdate",
        4096,
        NULL,
        3,
        &stateUpdateTaskHandle,
        0);
    
    ESP_LOGI(TAG, "Web server initialized");
}

/**
 * Start the web server
 */
void webserver_start(void) {
    ESP_LOGI(TAG, "Starting web server");
    
    // HTTP server configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = WEB_SERVER_PORT;
    config.max_uri_handlers = 10;
    config.stack_size = 8192;
    
    // Start the HTTP server
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Web server started on port %d", config.server_port);
        
        // Register URI handlers
        httpd_uri_t index = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &index);
        
        httpd_uri_t js = {
            .uri       = "/js/*",
            .method    = HTTP_GET,
            .handler   = js_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &js);
        
        httpd_uri_t css = {
            .uri       = "/css/*",
            .method    = HTTP_GET,
            .handler   = css_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &css);
        
        httpd_uri_t ws = {
            .uri       = WEBSOCKET_PATH,
            .method    = HTTP_GET,
            .handler   = websocket_handler,
            .user_ctx  = NULL,
            .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws);
        
        httpd_uri_t calibration = {
            .uri       = "/calibration",
            .method    = HTTP_GET,
            .handler   = calibration_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &calibration);
        
        httpd_uri_t api = {
            .uri       = "/api/*",
            .method    = HTTP_GET,
            .handler   = api_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &api);
        
        // Register OTA handlers
        webserver_register_ota_handlers(server);
    } else {
        ESP_LOGE(TAG, "Error starting web server");
    }
    
    // This function should not return
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * Stop the web server
 */
void webserver_stop(void) {
    if (server != NULL) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Web server stopped");
    }
}

/**
 * Send system state update to all connected WebSocket clients
 */
void webserver_send_state_update(void) {
    char json_buffer[512];
    generate_state_json(json_buffer, sizeof(json_buffer));
    broadcast_websocket_message(json_buffer);
}

/**
 * Check if any clients are connected
 */
bool webserver_has_clients(void) {
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
        if (wsClients[i].inUse) {
            return true;
        }
    }
    return false;
}

/**
 * Get the number of connected clients
 */
uint8_t webserver_get_client_count(void) {
    uint8_t count = 0;
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
        if (wsClients[i].inUse) {
            count++;
        }
    }
    return count;
}

/**
 * Register OTA update handlers
 */
esp_err_t webserver_register_ota_handlers(httpd_handle_t server) {
    // In a real implementation, you would register handlers for OTA updates here
    // For simplicity, we'll just log a message
    ESP_LOGI(TAG, "OTA update handlers would be registered here");
    return ESP_OK;
}

/**
 * Handler for the root path (/)
 */
static esp_err_t index_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving index.html");
    
    // Open the index.html file
    FILE *file = fopen("/spiffs/index.html", "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open index.html");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // Set content type
    httpd_resp_set_type(req, "text/html");
    
    // Read and send file in chunks
    char chunk[1024];
    size_t read_bytes;
    do {
        read_bytes = fread(chunk, 1, sizeof(chunk), file);
        if (read_bytes > 0) {
            httpd_resp_send_chunk(req, chunk, read_bytes);
        }
    } while (read_bytes > 0);
    
    // Close file and end response
    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}

/**
 * Handler for JavaScript files
 */
static esp_err_t js_handler(httpd_req_t *req) {
    // Extract the filename from the URI
    const char *filename = req->uri + 4; // Skip "/js/"
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "/spiffs/js/%s", filename);
    
    ESP_LOGI(TAG, "Serving %s", filepath);
    
    // Open the file
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open %s", filepath);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // Set content type
    httpd_resp_set_type(req, "application/javascript");
    
    // Read and send file in chunks
    char chunk[1024];
    size_t read_bytes;
    do {
        read_bytes = fread(chunk, 1, sizeof(chunk), file);
        if (read_bytes > 0) {
            httpd_resp_send_chunk(req, chunk, read_bytes);
        }
    } while (read_bytes > 0);
    
    // Close file and end response
    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}

/**
 * Handler for CSS files
 */
static esp_err_t css_handler(httpd_req_t *req) {
    // Extract the filename from the URI
    const char *filename = req->uri + 5; // Skip "/css/"
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "/spiffs/css/%s", filename);
    
    ESP_LOGI(TAG, "Serving %s", filepath);
    
    // Open the file
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open %s", filepath);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // Set content type
    httpd_resp_set_type(req, "text/css");
    
    // Read and send file in chunks
    char chunk[1024];
    size_t read_bytes;
    do {
        read_bytes = fread(chunk, 1, sizeof(chunk), file);
        if (read_bytes > 0) {
            httpd_resp_send_chunk(req, chunk, read_bytes);
        }
    } while (read_bytes > 0);
    
    // Close file and end response
    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}

/**
 * Handler for the calibration page
 */
static esp_err_t calibration_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving calibration.html");
    
    // Open the calibration.html file
    FILE *file = fopen("/spiffs/calibration.html", "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open calibration.html");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // Set content type
    httpd_resp_set_type(req, "text/html");
    
    // Read and send file in chunks
    char chunk[1024];
    size_t read_bytes;
    do {
        read_bytes = fread(chunk, 1, sizeof(chunk), file);
        if (read_bytes > 0) {
            httpd_resp_send_chunk(req, chunk, read_bytes);
        }
    } while (read_bytes > 0);
    
    // Close file and end response
    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}

/**
 * Handler for API endpoints
 */
static esp_err_t api_handler(httpd_req_t *req) {
    // Extract the endpoint from the URI
    const char *endpoint = req->uri + 5; // Skip "/api/"
    
    ESP_LOGI(TAG, "API request: %s", endpoint);
    
    // Handle different API endpoints
    if (strcmp(endpoint, "status") == 0) {
        // Return system status as JSON
        char json_buffer[512];
        generate_state_json(json_buffer, sizeof(json_buffer));
        
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json_buffer, strlen(json_buffer));
    } else if (strcmp(endpoint, "calibrate/motor") == 0) {
        // Calibrate motors
        bool success = motor_calibrate();
        
        char response[64];
        snprintf(response, sizeof(response), "{\"success\":%s}", success ? "true" : "false");
        
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
    } else if (strcmp(endpoint, "calibrate/battery") == 0) {
        // Get measured voltage from query parameter
        char voltage_str[16] = {0};
        size_t buf_len = httpd_req_get_url_query_len(req) + 1;
        if (buf_len > 1) {
            char *buf = malloc(buf_len);
            if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
                if (httpd_query_key_value(buf, "voltage", voltage_str, sizeof(voltage_str)) == ESP_OK) {
                    float measured_voltage = atof(voltage_str);
                    bool success = battery_calibrate(measured_voltage);
                    
                    char response[64];
                    snprintf(response, sizeof(response), "{\"success\":%s}", success ? "true" : "false");
                    
                    httpd_resp_set_type(req, "application/json");
                    httpd_resp_send(req, response, strlen(response));
                    free(buf);
                    return ESP_OK;
                }
            }
            free(buf);
        }
        
        // If we get here, there was an error
        httpd_resp_send_404(req);
    } else if (strcmp(endpoint, "reset") == 0) {
        // Reset failsafe
        bool success = failsafe_reset();
        
        char response[64];
        snprintf(response, sizeof(response), "{\"success\":%s}", success ? "true" : "false");
        
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
    } else {
        // Unknown endpoint
        httpd_resp_send_404(req);
    }
    
    return ESP_OK;
}

/**
 * WebSocket handler
 */
static esp_err_t websocket_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WebSocket handshake");
        
        // Find a free client slot
        int client_index = -1;
        for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
            if (!wsClients[i].inUse) {
                client_index = i;
                break;
            }
        }
        
        if (client_index == -1) {
            ESP_LOGW(TAG, "No free WebSocket client slots");
            return ESP_FAIL;
        }
        
        // Store client information
        wsClients[client_index].inUse = true;
        wsClients[client_index].fd = httpd_req_to_sockfd(req);
        wsClients[client_index].lastActivity = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        ESP_LOGI(TAG, "WebSocket client connected: %d", wsClients[client_index].fd);
        
        // Send initial state
        char json_buffer[512];
        generate_state_json(json_buffer, sizeof(json_buffer));
        send_websocket_message(wsClients[client_index].fd, json_buffer);
    }
    
    return ESP_OK;
}

/**
 * WebSocket frame receive callback
 */
esp_err_t websocket_recv_cb(httpd_handle_t handle, httpd_req_t *req, httpd_ws_frame_t *frame, void *user_ctx) {
    // Find the client
    int client_fd = httpd_req_to_sockfd(req);
    int client_index = -1;
    
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
        if (wsClients[i].inUse && wsClients[i].fd == client_fd) {
            client_index = i;
            break;
        }
    }
    
    if (client_index == -1) {
        ESP_LOGW(TAG, "WebSocket message from unknown client: %d", client_fd);
        return ESP_FAIL;
    }
    
    // Update last activity time
    wsClients[client_index].lastActivity = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Handle different frame types
    if (frame->type == HTTPD_WS_TYPE_TEXT) {
        // Process text message
        char *message = malloc(frame->len + 1);
        if (message == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory for WebSocket message");
            return ESP_FAIL;
        }
        
        memcpy(message, frame->payload, frame->len);
        message[frame->len] = '\0';
        
        ESP_LOGI(TAG, "WebSocket message: %s", message);
        
        // Process the message
        process_websocket_message(client_fd, message, frame->len);
        
        free(message);
    } else if (frame->type == HTTPD_WS_TYPE_BINARY) {
        // Process binary message (not implemented)
        ESP_LOGW(TAG, "WebSocket binary message not implemented");
    } else if (frame->type == HTTPD_WS_TYPE_CLOSE) {
        // Client disconnected
        ESP_LOGI(TAG, "WebSocket client disconnected: %d", client_fd);
        wsClients[client_index].inUse = false;
        wsClients[client_index].fd = -1;
    }
    
    return ESP_OK;
}

/**
 * Process a WebSocket message
 */
static void process_websocket_message(int client_fd, const char *msg, size_t len) {
    // Parse JSON message
    cJSON *root = cJSON_Parse(msg);
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse WebSocket message as JSON");
        return;
    }
    
    // Check message type
    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (type == NULL || !cJSON_IsString(type)) {
        ESP_LOGE(TAG, "WebSocket message missing 'type' field");
        cJSON_Delete(root);
        return;
    }
    
    // Handle different message types
    if (strcmp(type->valuestring, "control") == 0) {
        // Control message
        cJSON *left = cJSON_GetObjectItem(root, "left");
        cJSON *right = cJSON_GetObjectItem(root, "right");
        
        if (left != NULL && cJSON_IsNumber(left) && 
            right != NULL && cJSON_IsNumber(right)) {
            // Update motor commands
            systemState->leftMotorCommand = left->valueint;
            systemState->rightMotorCommand = right->valueint;
            
            // Update failsafe command time
            failsafe_update_command_time();
            
            ESP_LOGI(TAG, "Motor control: left=%d, right=%d", 
                     systemState->leftMotorCommand, systemState->rightMotorCommand);
        }
    } else if (strcmp(type->valuestring, "lights") == 0) {
        // Lights message
        cJSON *headlights = cJSON_GetObjectItem(root, "headlights");
        cJSON *taillights = cJSON_GetObjectItem(root, "taillights");
        cJSON *indicators = cJSON_GetObjectItem(root, "indicators");
        
        if (headlights != NULL && cJSON_IsBool(headlights)) {
            lights_set_headlights(cJSON_IsTrue(headlights));
            systemState->headlightsOn = cJSON_IsTrue(headlights);
        }
        
        if (taillights != NULL && cJSON_IsBool(taillights)) {
            lights_set_taillights(cJSON_IsTrue(taillights));
            systemState->taillightsOn = cJSON_IsTrue(taillights);
        }
        
        if (indicators != NULL && cJSON_IsNumber(indicators)) {
            lights_set_indicators(indicators->valueint);
            systemState->indicatorState = indicators->valueint;
        }
        
        // Update failsafe command time
        failsafe_update_command_time();
    } else if (strcmp(type->valuestring, "horn") == 0) {
        // Horn message
        cJSON *state = cJSON_GetObjectItem(root, "state");
        
        if (state != NULL && cJSON_IsBool(state)) {
            // Set horn state (in a real implementation, you would control a GPIO pin)
            systemState->hornOn = cJSON_IsTrue(state);
            
            // For now, just log the horn state
            ESP_LOGI(TAG, "Horn %s", systemState->hornOn ? "ON" : "OFF");
            
            // In a real implementation, you would control the horn here
            // gpio_set_level(HORN_PIN, systemState->hornOn ? 1 : 0);
            
            // Update failsafe command time
            failsafe_update_command_time();
        }
    } else if (strcmp(type->valuestring, "underglow") == 0) {
        // Underglow message
        cJSON *r = cJSON_GetObjectItem(root, "r");
        cJSON *g = cJSON_GetObjectItem(root, "g");
        cJSON *b = cJSON_GetObjectItem(root, "b");
        cJSON *mode = cJSON_GetObjectItem(root, "mode");
        
        if (r != NULL && cJSON_IsNumber(r) && 
            g != NULL && cJSON_IsNumber(g) && 
            b != NULL && cJSON_IsNumber(b)) {
            // Set underglow color
            lights_set_underglow(r->valueint, g->valueint, b->valueint);
            systemState->underglowR = r->valueint;
            systemState->underglowG = g->valueint;
            systemState->underglowB = b->valueint;
        }
        
        if (mode != NULL && cJSON_IsNumber(mode)) {
            // Set underglow mode
            lights_set_underglow_mode(mode->valueint);
        }
        
        // Update failsafe command time
        failsafe_update_command_time();
    }
    
    cJSON_Delete(root);
}

/**
 * Send a WebSocket message to a client
 */
static void send_websocket_message(int client_fd, const char *msg) {
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)msg;
    ws_pkt.len = strlen(msg);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    httpd_ws_send_frame_async(server, client_fd, &ws_pkt);
}

/**
 * Broadcast a WebSocket message to all connected clients
 */
static void broadcast_websocket_message(const char *msg) {
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
        if (wsClients[i].inUse) {
            send_websocket_message(wsClients[i].fd, msg);
        }
    }
}

/**
 * Generate JSON representation of system state
 */
static void generate_state_json(char *buffer, size_t buffer_size) {
    // Create JSON object
    cJSON *root = cJSON_CreateObject();
    
    // Add system info
    cJSON_AddStringToObject(root, "version", FIRMWARE_VERSION);
    cJSON_AddNumberToObject(root, "uptime", systemState->uptime);
    
    // Add motor state
    cJSON *motors = cJSON_CreateObject();
    cJSON_AddNumberToObject(motors, "leftSpeed", systemState->leftMotorSpeed);
    cJSON_AddNumberToObject(motors, "rightSpeed", systemState->rightMotorSpeed);
    cJSON_AddItemToObject(root, "motors", motors);
    
    // Add light state
    cJSON *lights = cJSON_CreateObject();
    cJSON_AddBoolToObject(lights, "headlights", systemState->headlightsOn);
    cJSON_AddBoolToObject(lights, "taillights", systemState->taillightsOn);
    cJSON_AddNumberToObject(lights, "indicators", systemState->indicatorState);
    
    // Add underglow state
    cJSON *underglow = cJSON_CreateObject();
    cJSON_AddNumberToObject(underglow, "r", systemState->underglowR);
    cJSON_AddNumberToObject(underglow, "g", systemState->underglowG);
    cJSON_AddNumberToObject(underglow, "b", systemState->underglowB);
    cJSON_AddItemToObject(lights, "underglow", underglow);
    
    cJSON_AddItemToObject(root, "lights", lights);
    
    // Add horn state
    cJSON_AddBoolToObject(root, "horn", systemState->hornOn);
    
    // Add battery state
    cJSON *battery = cJSON_CreateObject();
    cJSON_AddNumberToObject(battery, "voltage", systemState->batteryVoltage);
    cJSON_AddNumberToObject(battery, "percentage", systemState->batteryPercentage);
    cJSON_AddBoolToObject(battery, "low", systemState->batteryLow);
    cJSON_AddBoolToObject(battery, "critical", systemState->batteryCritical);
    cJSON_AddItemToObject(root, "battery", battery);
    
    // Add failsafe state
    cJSON *failsafe = cJSON_CreateObject();
    cJSON_AddBoolToObject(failsafe, "active", systemState->failsafeActive);
    cJSON_AddNumberToObject(failsafe, "reason", systemState->failsafeReason);
    cJSON_AddItemToObject(root, "failsafe", failsafe);
    
    // Add connection state
    cJSON_AddBoolToObject(root, "connected", systemState->connectionActive);
    
    // Generate JSON string
    char *json_str = cJSON_PrintUnformatted(root);
    if (json_str) {
        strncpy(buffer, json_str, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        free(json_str);
    } else {
        snprintf(buffer, buffer_size, "{\"error\":\"Failed to generate JSON\"}");
    }
    
    cJSON_Delete(root);
}

/**
 * State update task
 * Periodically sends system state updates to connected WebSocket clients
 */
static void state_update_task(void *pvParameters) {
    ESP_LOGI(TAG, "State update task started on core %d", xPortGetCoreID());
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(WEBSOCKET_UPDATE_INTERVAL_MS);
    
    while (1) {
        // Update system uptime
        systemState->uptime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        
        // Send state update to all connected clients
        if (webserver_has_clients()) {
            webserver_send_state_update();
        }
        
        // Check for inactive clients
        uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
        for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++) {
            if (wsClients[i].inUse && 
                (currentTime - wsClients[i].lastActivity > 30000)) { // 30 seconds timeout
                ESP_LOGW(TAG, "WebSocket client %d timed out", wsClients[i].fd);
                wsClients[i].inUse = false;
                wsClients[i].fd = -1;
            }
        }
        
        // Delay until the next update
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}