#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// WiFi credentials
const char *ssid = "Ravi4G";
const char *password = "Ravi4321";

// Server and WebSocket setup
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Touch setup
const int touchPin = T0; // GPIO 4
const int touchThreshold = 30;

bool lastTouch = false;
unsigned long lastWiFiCheck = 0;

// Send message to WebSocket clients
void notifyBlink()
{
  ws.textAll("blink");
}

void setup()
{
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  // Serve HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  // Setup WebSocket
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len)
             {
               if (type == WS_EVT_CONNECT)
               {
                 Serial.println("WebSocket client connected");
               }
               else if (type == WS_EVT_DISCONNECT)
               {
                 Serial.println("WebSocket client disconnected");
               } });
  server.addHandler(&ws);

  server.begin();
  Serial.println("Server started");
}

void loop()
{
  // Read touch
  int t = touchRead(touchPin);
  bool isTouched = t < touchThreshold;

  // On new touch, notify via WebSocket
  if (isTouched && !lastTouch)
  {
    Serial.println("Touch detected: blink!");
    notifyBlink();
  }

  lastTouch = isTouched;

  // Re-check WiFi every 5 seconds
  if (millis() - lastWiFiCheck > 5000)
  {
    lastWiFiCheck = millis();
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi disconnected! Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    else
    {
      Serial.println("WiFi OK. IP: " + WiFi.localIP().toString());
    }
  }

  delay(50);
}
