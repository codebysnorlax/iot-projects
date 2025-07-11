#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

extern int SPEED;
extern int speed_Coeff;

void initMotors();
void Forward();
void Backward();
void TurnRight();
void TurnLeft();
void ForwardRight();
void ForwardLeft();
void BackwardRight();
void BackwardLeft();
void Stop();

#endif