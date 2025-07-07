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
const int OPEN_SPEED    = 3; // Faster speed for opening
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
void blinkAllLeds(int times, int duration);
void playOpeningTune();

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
  Serial.println("Smart Dustbin Initialized");
}

void loop() {
  unsigned long now = millis();
  int distance = getDistance();
  Serial.print("Distance: ");
  Serial.println(distance);

  switch (currentState) {
    case CLOSED:
      Serial.println("State: CLOSED");
      digitalWrite(blueLed, HIGH);
      smoothDimToBright(redLed, 2000);
      digitalWrite(greenLed, LOW);
      
      if (distance > 0 && distance <= 35) {
        Serial.println("Object detected, opening bin...");
        playOpeningTune();
        currentState = OPENING;
        myServo.attach(servoPin);
      }
      break;

    case OPENING:
      Serial.println("State: OPENING");
      digitalWrite(blueLed, LOW);
      digitalWrite(redLed, LOW);
      if ((now / 100) % 2 == 0) {
        digitalWrite(greenLed, HIGH);
      } else {
        digitalWrite(greenLed, LOW);
      }
      if (currentServoPos > OPEN_POS) {
        myServo.write(currentServoPos -= OPEN_SPEED);
        delay(SERVO_STEP_DELAY);
      } else {
        Serial.println("Bin opened");
        currentState = OPEN;
        digitalWrite(greenLed, HIGH);
      }
      break;

    case OPEN:
      Serial.println("State: OPEN");
      delay(5000);
      Serial.println("Closing soon...");
      greenWarningWithBeep();
      currentState = CLOSING;
      break;

    case CLOSING:
      Serial.println("State: CLOSING");
      digitalWrite(greenLed, LOW);
      if ((now / 300) % 2 == 0) {
        digitalWrite(redLed, HIGH);
      } else {
        digitalWrite(redLed, LOW);
      }
      if (distance > 0 && distance <= 20) {
        Serial.println("Object detected, reopening bin...");
        currentState = OPENING;
      } else if (currentServoPos < CLOSED_POS) {
        myServo.write(currentServoPos += CLOSE_SPEED);
        delay(SERVO_STEP_DELAY);
      } else {
        Serial.println("Bin closed");
        blinkAllLeds(2, 400);
        currentState = CLOSED;
        myServo.detach();
        digitalWrite(redLed, LOW);
        digitalWrite(greenLed, LOW);
        digitalWrite(blueLed, HIGH);
      }
      break;
  }
}

// 🎵 Function to Play Soft Music Before Opening
void playOpeningTune() {
  Serial.println("Playing opening tune");
  int melody[] = {262, 330, 392, 523};
  int duration[] = {200, 200, 300, 400};

  for (int i = 0; i < 4; i++) {
    tone(buzzer, melody[i], duration[i]);
    delay(duration[i] * 1.2);
  }
  noTone(buzzer);
}

// 📏 Function to Get Distance from Ultrasonic Sensor
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH) * 0.034 / 2;
}

// 🔔 Smooth Warning Before Closing
void greenWarningWithBeep() {
  Serial.println("Warning: Bin closing soon");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 255; j += 25) {
      analogWrite(greenLed, j);
      delay(10);
    }
    tone(buzzer, 700, 200);
    delay(200);
    for (int j = 255; j >= 0; j -= 25) {
      analogWrite(greenLed, j);
      delay(10);
    }
    delay(150);
  }
  noTone(buzzer);
  digitalWrite(greenLed, LOW);
}

// 💡 Smoothly Dims and Brightens the Red LED
void smoothDimToBright(int ledPin, unsigned long cycleDuration) {
  unsigned long halfCycle = cycleDuration / 2;
  unsigned long currentTime = millis() % cycleDuration;
  int brightness = map(currentTime, 0, halfCycle, 0, 255);
  if (currentTime > halfCycle) {
    brightness = map(currentTime, halfCycle, cycleDuration, 255, 0);
  }
  analogWrite(ledPin, brightness);
}

// ✨ Blink All LEDs
void blinkAllLeds(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, HIGH);
    digitalWrite(blueLed, HIGH);
    delay(duration / 2);
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, LOW);
    digitalWrite(blueLed, LOW);
    delay(duration / 2);
  }
}
