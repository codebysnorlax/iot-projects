#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <config.h>

// WiFi credentials
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;

// Telegram bot token and chat ID
String botToken = BOT_TOKEN;
String chatId = CHAT_ID; // Your chat ID

unsigned long previousMillis = 0;
const long interval = 5000; // 5 seconds

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClientSecure client;
            client.setInsecure(); // This skips certificate validation, quick and works.

            HTTPClient http;

            String message = "Hello from ESP32";
            String url = "https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatId + "&text=" + message;

            Serial.println("Sending message to: " + url);

            http.begin(client, url); // Use secure client
            int httpResponseCode = http.GET();

            if (httpResponseCode > 0)
            {
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);

                String response = http.getString();
                Serial.println("Response payload: " + response);
            }
            else
            {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }

            http.end(); // Properly close connection
        }
        else
        {
            Serial.println("WiFi not connected, attempting reconnect...");
            WiFi.reconnect();
        }
    }
}
