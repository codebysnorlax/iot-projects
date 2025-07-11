#include <Arduino.h>
#include "motor_control.h"

extern int enA, enB, in1, in2, in3, in4;
int SPEED = 1023;
int speed_Coeff = 3;

void initMotors() {
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    Stop();
}

void Forward() {
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void Backward() {
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void TurnRight() {
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void TurnLeft() {
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED);
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void ForwardRight() {
    analogWrite(enA, SPEED / speed_Coeff);
    analogWrite(enB, SPEED);
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void ForwardLeft() {
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED / speed_Coeff);
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void BackwardRight() {
    analogWrite(enA, SPEED / speed_Coeff);
    analogWrite(enB, SPEED);
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void BackwardLeft() {
    analogWrite(enA, SPEED);
    analogWrite(enB, SPEED / speed_Coeff);
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void Stop() {
    analogWrite(enA, 0);
    analogWrite(enB, 0);
    digitalWrite(in1, LOW); digitalWrite(in2, LOW);
    digitalWrite(in3, LOW); digitalWrite(in4, LOW);
}