#include <Arduino.h>
#include "buzzer_led.h"

extern int buzPin;
extern int ledPin;

void initBuzzLed() {
    pinMode(buzPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(buzPin, LOW);
    digitalWrite(ledPin, LOW);
}

void BeepOn() {
    digitalWrite(buzPin, HIGH);
    digitalWrite(ledPin, HIGH);
}

void BeepOff() {
    digitalWrite(buzPin, LOW);
    digitalWrite(ledPin, LOW);
}