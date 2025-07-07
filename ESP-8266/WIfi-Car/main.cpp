#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

// Motor control pins
int enA = D1; // Right motor speed
int in1 = D2; // Right motor forward
int in2 = D3; // Right motor backward
int in3 = D4; // Left motor forward
int in4 = D5; // Left motor backward
int enB = D6; // Left motor speed

const int buzPin = D7;     // Active buzzer
const int ledPin = D8;     // Status LED
const int wifiLedPin = D0; // WiFi status LED

String command;      // Stores incoming commands
int SPEED = 1023;    // Default speed (PWM range 0-1023)
int speed_Coeff = 3; // Speed coefficient for diagonal moves

ESP8266WebServer server(80); // Web server on port 80

unsigned long previousMillis = 0;

String sta_ssid = "Trash Car";     // WiFi network to connect to
String sta_password = "Trash8266"; // WiFi password

// Function prototypes
void HTTP_handleRoot(void);
void handleNotFound();
void Forward();
void Backward();
void TurnRight();
void TurnLeft();
void ForwardRight();
void ForwardLeft();
void BackwardRight();
void BackwardLeft();
void Stop();
void BeepOn();
void BeepOff();

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("**WiFi Robot Remote Control Mode - L298N 2A**");
    Serial.println("---------------------------");

    // Initialize pins
    pinMode(buzPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(wifiLedPin, OUTPUT);
    digitalWrite(buzPin, LOW);
    digitalWrite(ledPin, LOW);
    digitalWrite(wifiLedPin, HIGH);

    // Motor control pins
    pinMode(enA, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    pinMode(enB, OUTPUT);

    // Initial motor state
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);

    // Set hostname based on chip ID
    String chip_id = String(ESP.getChipId(), HEX);
    int i = chip_id.length() - 4;
    chip_id = chip_id.substring(i);
    chip_id = "wificar-" + chip_id;
    String hostname(chip_id);

    Serial.println();
    Serial.println("Hostname: " + hostname);

    // Try to connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
    Serial.print("Connecting to ");
    Serial.print(sta_ssid);
    Serial.print("...");

    // Wait for connection
    unsigned long currentMillis = millis();
    previousMillis = currentMillis;
    while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 10000)
    {
        delay(500);
        Serial.print(".");
        currentMillis = millis();
    }

    // Connection result
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("WiFi-STA-Mode");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(wifiLedPin, LOW); // LED on when connected
        delay(3000);
    }
    else
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(hostname.c_str());
        IPAddress myIP = WiFi.softAPIP();
        Serial.println("WiFi failed to connect to " + sta_ssid);
        Serial.println("");
        Serial.println("WiFi-AP-Mode");
        Serial.print("AP IP address: ");
        Serial.println(myIP);
        digitalWrite(wifiLedPin, HIGH); // LED off in AP mode
        delay(3000);
    }

    // Set up web server
    server.on("/", HTTP_handleRoot);
    server.onNotFound(HTTP_handleRoot);
    server.begin();

    // Enable OTA updates
    ArduinoOTA.begin();
}

void loop()
{
    ArduinoOTA.handle();
    server.handleClient();

    command = server.arg("State");

    // Process commands
    if (command == "e")
        Forward();
    else if (command == "b")
        Backward();
    else if (command == "r")
        TurnRight();
    else if (command == "l")
        TurnLeft();
    else if (command == "s")
        Stop();
    else if (command == "fr")
        ForwardRight();
    else if (command == "fl")
        ForwardLeft();
    else if (command == "br")
        BackwardRight();
    else if (command == "bl")
        BackwardLeft();
    else if (command == "f1")
        BeepOn();
    else if (command == "f0")
        BeepOff();
    else if (command == "0")
        SPEED = 0;
    else if (command == "1")
        SPEED = 330;
    else if (command == "2")
        SPEED = 470;
    else if (command == "3")
        SPEED = 530;
    else if (command == "4")
        SPEED = 670;
    else if (command == "5")
        SPEED = 730;
    else if (command == "6")
        SPEED = 870;
    else if (command == "7")
        SPEED = 930;
    else if (command == "8")
        SPEED = 1000;
    else if (command == "9")
        SPEED = 1023;
    else if (command == "q")
        SPEED = 1023;
}

// Web server handlers
void HTTP_handleRoot(void)
{
    server.send(200, "text/html", " ");
    if (server.hasArg("State"))
    {
        Serial.println(server.arg("State"));
    }
}

void handleNotFound()
{
    server.send(404, "text/plain", "404: Not Found");
}

// Movement functions
void Forward()
{
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

void Backward()
{
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

void TurnRight()
{
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

void TurnLeft()
{
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

void ForwardRight()
{
    analogWrite(enA, SPEED / speed_Coeff);
    analogWrite(enB, SPEED);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

void ForwardLeft()
{
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED / speed_Coeff);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

void BackwardRight()
{
    analogWrite(enA, SPEED / speed_Coeff);
    analogWrite(enB, SPEED);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

void BackwardLeft()
{
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED / speed_Coeff);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

void Stop()
{
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    analogWrite(enA, 0);
    analogWrite(enB, 0);
}

// Buzzer and LED control
void BeepOn()
{
    digitalWrite(buzPin, HIGH);
    digitalWrite(ledPin, HIGH);
}

void BeepOff()
{
    digitalWrite(buzPin, LOW);
    digitalWrite(ledPin, LOW);
}