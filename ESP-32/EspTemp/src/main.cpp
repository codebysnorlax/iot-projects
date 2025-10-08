#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>

// WiFi credentials
const char *ssid = "Ravi4G";
const char *password = "Ravi4321";

// DS18B20 Configuration
#define ONE_WIRE_BUS 4
#define TEMP_UPDATE_INTERVAL 100 // Update every 0.1 seconds (100ms)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Web server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variables
unsigned long lastTempUpdate = 0;
float lastTemperature = 0.0;
bool sensorError = false;

// Get temperature with error handling
String getTemperature()
{
  static unsigned long lastRequest = 0;
  static bool conversionStarted = false;

  // Start conversion if not already started
  if (!conversionStarted || millis() - lastRequest > 100)
  {
    sensors.requestTemperatures();
    conversionStarted = true;
    lastRequest = millis();
  }

  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C || tempC < -55 || tempC > 125)
  {
    sensorError = true;
    return "Error";
  }

  sensorError = false;
  lastTemperature = tempC;
  return String(tempC, 1); // Reduced precision for faster processing
}

// Send temperature via WebSocket (JSON format for better handling)
void notifyClients()
{
  String temp = getTemperature();
  String message;

  if (sensorError)
  {
    message = "{\"temperature\":\"Error\",\"status\":\"error\"}";
  }
  else
  {
    message = "{\"temperature\":" + temp + ",\"status\":\"ok\"}";
  }

  ws.textAll(message);
  Serial.println("Sent: " + message);
}

// WebSocket event handler
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n",
                  client->id(), client->remoteIP().toString().c_str());
    // Send current temperature immediately to new client
    client->text(sensorError ? "{\"temperature\":\"Error\",\"status\":\"error\"}" : "{\"temperature\":" + String(lastTemperature, 2) + ",\"status\":\"ok\"}");
    break;

  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;

  case WS_EVT_DATA:
    // Handle incoming WebSocket messages if needed
    break;

  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== ESP32 Temperature Monitor ===");

  // Init DS18B20
  sensors.begin();
  int deviceCount = sensors.getDeviceCount();
  Serial.printf("Found %d DS18B20 sensor(s)\n", deviceCount);

  if (deviceCount == 0)
  {
    Serial.println("WARNING: No DS18B20 sensors detected!");
  }

  // Set sensor resolution (9 bits for fastest response)
  sensors.setResolution(9);            // 9-bit = 93.75ms, 10-bit = 187.5ms, 11-bit = 375ms, 12-bit = 750ms
  sensors.setWaitForConversion(false); // Don't block waiting for conversion

  // Init SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("ERROR: SPIFFS mount failed!");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("\nERROR: WiFi connection failed!");
    return;
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // Setup WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // Serve index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  // API endpoint for temperature (REST)
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String temp = getTemperature();
    String json;

    if (sensorError) {
      json = "{\"temperature\":\"Error\",\"status\":\"error\"}";
      request->send(500, "application/json", json);
    } else {
      json = "{\"temperature\":" + temp + ",\"status\":\"ok\"}";
      request->send(200, "application/json", json);
    } });

  // Handle 404
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "Not found"); });

  // Start server
  server.begin();
  Serial.println("Web server started");
  Serial.println("================================\n");
}

void loop()
{
  // Cleanup disconnected WebSocket clients (less frequently)
  static unsigned long lastCleanup = 0;
  if (millis() - lastCleanup > 1000)
  {
    ws.cleanupClients();
    lastCleanup = millis();
  }

  // Send temperature updates every 100ms
  if (millis() - lastTempUpdate >= TEMP_UPDATE_INTERVAL)
  {
    notifyClients();
    lastTempUpdate = millis();
  }

  // Minimal delay
  yield();
}
