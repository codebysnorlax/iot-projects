#include <Arduino.h>

#define LED_BUILTIN 2 // On-board LED for most ESP32 boards

// Morse code timing (in milliseconds)
const int shortBlink = 200;  // Duration of a dot
const int longBlink = 600;   // Duration of a dash
const int symbolSpace = 200; // Space between dots and dashes
const int letterSpace = 600; // Space between letters
const int wordSpace = 1400;  // Space between words

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
}

void dot()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(shortBlink); 
  digitalWrite(LED_BUILTIN, LOW);
  delay(symbolSpace);
}

void dash()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(longBlink);
  digitalWrite(LED_BUILTIN, LOW);
  delay(symbolSpace);
}

void loop()
{
  dot();
  dot();
  dot();
  delay(letterSpace);

  dash();
  dash();
  dash();
  delay(letterSpace);

  dot();
  dot();
  dot();
  delay(wordSpace);
}