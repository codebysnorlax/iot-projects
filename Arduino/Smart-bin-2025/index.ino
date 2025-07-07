#include <Servo.h>

// Pin Definitions
const int trigPin   = 9;
const int echoPin   = 10;
const int redLed    = 3;  // PWM pin for dim-to-bright effect
const int greenLed  = 4;
const int buzzer    = 5;
const int servoPin  = 6;
const int blueLed   = 7;

// Servo Movement Parameters
const int OPEN_POS      = 0;
const int CLOSED_POS    = 140;
const int OPEN_SPEED    = 3;  // Faster speed for opening
const int CLOSE_SPEED   = 1;  // Slower speed for closing
const unsigned long SERVO_STEP_DELAY = 15; // ms between servo updates

// State Machine Definitions
enum BinState { CLOSED, OPENING, OPEN, CLOSING };
BinState currentState = CLOSED;
int currentServoPos = CLOSED_POS;
unsigned long lastServoStepTime = 0;

// Function Prototypes
int getDistance();
void greenWarningWithBeep();
void smoothDimToBright(int ledPin, unsigned long cycleDuration);

Servo myServo;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(blueLed, OUTPUT);
  
  myServo.attach(servoPin);
  myServo.write(CLOSED_POS);
  myServo.detach();
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, HIGH);
}

void loop() {
  unsigned long now = millis();
  int distance = getDistance();

  switch (currentState) {
    case CLOSED:
      digitalWrite(blueLed, HIGH);
      smoothDimToBright(redLed, 2000); // Red LED dim-to-bright effect (2-second cycle)
      digitalWrite(greenLed, LOW);
      if (distance > 0 && distance <= 35) {
        currentState = OPENING;
        myServo.attach(servoPin);
      }
      break;

    case OPENING:
      digitalWrite(blueLed, LOW);
      digitalWrite(redLed, LOW);
      // Green LED blinks every 0.4 seconds
      if ((now / 100) % 2 == 0) {
        digitalWrite(greenLed, HIGH);
      } else {
        digitalWrite(greenLed, LOW);
      }
      if (currentServoPos > OPEN_POS) {
        myServo.write(currentServoPos -= OPEN_SPEED);
        delay(SERVO_STEP_DELAY);
      } else {
        currentState = OPEN;
        digitalWrite(greenLed, HIGH); // Green LED fully on when fully opened
      }
      break;

    case OPEN:
      delay(5000); // Lid stays open for 5 seconds
      greenWarningWithBeep(); // Smooth, pleasant warning before closing
      currentState = CLOSING;
      break;

    case CLOSING:
      digitalWrite(greenLed, LOW);
      // Red LED blinks every 0.4 seconds
      if ((now / 300) % 2 == 0) {
        digitalWrite(redLed, HIGH);
      } else {
        digitalWrite(redLed, LOW);
      }
      if (distance > 0 && distance <= 20) {
        currentState = OPENING; // Reopen if something is detected while closing
      } else if (currentServoPos < CLOSED_POS) {
        myServo.write(currentServoPos += CLOSE_SPEED);
        delay(SERVO_STEP_DELAY);
      } else {
        currentState = CLOSED;
        myServo.detach();
        digitalWrite(redLed, LOW);
      }
      break;
  }
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH) * 0.034 / 2;
}

void greenWarningWithBeep() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 255; j += 25) { // Smooth fade-in effect
      analogWrite(greenLed, j);
      delay(10);
    }
    tone(buzzer, 700, 200); // Softer, calmer beep
    delay(200);
    for (int j = 255; j >= 0; j -= 25) { // Smooth fade-out effect
      analogWrite(greenLed, j);
      delay(10);
    }
    delay(150); // Short pause between blinks
  }
  noTone(buzzer);
  digitalWrite(greenLed, LOW);
}

void smoothDimToBright(int ledPin, unsigned long cycleDuration) {
  unsigned long halfCycle = cycleDuration / 2;
  unsigned long currentTime = millis() % cycleDuration;
  int brightness = map(currentTime, 0, halfCycle, 0, 255); // Dim to bright
  if (currentTime > halfCycle) {
    brightness = map(currentTime, halfCycle, cycleDuration, 255, 0); // Bright to dim
  }
  analogWrite(ledPin, brightness);
}

