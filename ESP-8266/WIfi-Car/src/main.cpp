#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include "motor_control.h"
#include "buzzer_led.h"
#include "web_server.h"

int enA = D1, in1 = D2, in2 = D3, in3 = D4, in4 = D5, enB = D6;
int buzPin = D7, ledPin = D8, wifiLedPin = D0;

ESP8266WebServer server(80);
String command;
String sta_ssid = "Trash Car", sta_password = "Trash8266";
unsigned long previousMillis = 0;

void setup() {
    Serial.begin(115200);
    pinMode(wifiLedPin, OUTPUT);
    digitalWrite(wifiLedPin, HIGH);

    initMotors();
    initBuzzLed();

    WiFi.mode(WIFI_STA);
    WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
    Serial.print("Connecting to ");
    Serial.print(sta_ssid);
    Serial.print("...");

    unsigned long currentMillis = millis();
    previousMillis = currentMillis;
    while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 10000) {
        delay(500);
        Serial.print(".");
        currentMillis = millis();
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi-STA-Mode");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(wifiLedPin, LOW);
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ESP8266-AP");
        Serial.println("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }

    server.on("/", HTTP_handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();
    ArduinoOTA.begin();
}

void loop() {
    ArduinoOTA.handle();
    server.handleClient();

    command = server.arg("State");
    if (command == "e") Forward();
    else if (command == "b") Backward();
    else if (command == "r") TurnRight();
    else if (command == "l") TurnLeft();
    else if (command == "s") Stop();
    else if (command == "fr") ForwardRight();
    else if (command == "fl") ForwardLeft();
    else if (command == "br") BackwardRight();
    else if (command == "bl") BackwardLeft();
    else if (command == "f1") BeepOn();
    else if (command == "f0") BeepOff();
    else if (command >= "0" && command <= "9") SPEED = map(command[0] - '0', 0, 9, 0, 1023);
    else if (command == "q") SPEED = 1023;
}