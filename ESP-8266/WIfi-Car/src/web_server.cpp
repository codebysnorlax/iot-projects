#include <ESP8266WebServer.h>
#include "web_server.h"

extern ESP8266WebServer server;

void HTTP_handleRoot() {
    server.send(200, "text/html", " ");
    if (server.hasArg("State")) {
        Serial.println(server.arg("State"));
    }
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Not Found");
}