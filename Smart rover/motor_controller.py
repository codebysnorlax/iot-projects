# Motor Controller Module for Smart Rover
# Handles 4 DC motors with L298N driver in tank-style configuration

from machine import Pin, PWM
import time

class MotorController:
    def __init__(self):
        # Pin Configuration for L298N
        # Motor A - Left Side Front
        self.motor_a_en = PWM(Pin(5))  # D1 - Enable/Speed control
        self.motor_a_in1 = Pin(4, Pin.OUT)  # D2 - Direction control 1
        self.motor_a_in2 = Pin(0, Pin.OUT)  # D3 - Direction control 2
        
        # Motor B - Left Side Rear
        self.motor_b_en = PWM(Pin(2))  # D4 - Enable/Speed control
        self.motor_b_in1 = Pin(14, Pin.OUT)  # D5 - Direction control 1
        self.motor_b_in2 = Pin(12, Pin.OUT)  # D6 - Direction control 2
        
        # Motor C - Right Side Front
        self.motor_c_en = PWM(Pin(13))  # D7 - Enable/Speed control
        self.motor_c_in1 = Pin(15, Pin.OUT)  # D8 - Direction control 1
        self.motor_c_in2 = Pin(3, Pin.OUT)  # RX - Direction control 2
        
        # Motor D - Right Side Rear
        self.motor_d_en = PWM(Pin(1))  # TX - Enable/Speed control
        self.motor_d_in1 = Pin(16, Pin.OUT)  # D0 - Direction control 1
        self.motor_d_in2 = Pin(10, Pin.OUT)  # SD3 - Direction control 2
        
        # Initialize PWM for speed control (0-1023)
        self.motor_a_en.freq(1000)
        self.motor_b_en.freq(1000)
        self.motor_c_en.freq(1000)
        self.motor_d_en.freq(1000)
        
        # Set initial speed (0-1023)
        self.speed = 800
        self.set_speed(self.speed)
        
        # Stop all motors initially
        self.stop()
        
        # Track last command time for safety timeout
        self.last_command_time = time.time()
        self.timeout_duration = 2  # seconds
        
        print("Motor Controller initialized")
    
    def set_speed(self, speed):
        """Set speed for all motors (0-1023)"""
        self.speed = max(0, min(1023, speed))
        self.motor_a_en.duty(self.speed)
        self.motor_b_en.duty(self.speed)
        self.motor_c_en.duty(self.speed)
        self.motor_d_en.duty(self.speed)
        return self.speed
    
    def _update_command_time(self):
        """Update the last command time for timeout safety"""
        self.last_command_time = time.time()
    
    def forward(self):
        """Move rover forward"""
        # Left side motors
        self.motor_a_in1.value(1)
        self.motor_a_in2.value(0)
        self.motor_b_in1.value(1)
        self.motor_b_in2.value(0)
        
        # Right side motors
        self.motor_c_in1.value(1)
        self.motor_c_in2.value(0)
        self.motor_d_in1.value(1)
        self.motor_d_in2.value(0)
        
        self._update_command_time()
        return "forward"
    
    def backward(self):
        """Move rover backward"""
        # Left side motors
        self.motor_a_in1.value(0)
        self.motor_a_in2.value(1)
        self.motor_b_in1.value(0)
        self.motor_b_in2.value(1)
        
        # Right side motors
        self.motor_c_in1.value(0)
        self.motor_c_in2.value(1)
        self.motor_d_in1.value(0)
        self.motor_d_in2.value(1)
        
        self._update_command_time()
        return "backward"
    
    def turn_left(self):
        """Tank-style left turn (left motors backward, right motors forward)"""
        # Left side motors - backward
        self.motor_a_in1.value(0)
        self.motor_a_in2.value(1)
        self.motor_b_in1.value(0)
        self.motor_b_in2.value(1)
        
        # Right side motors - forward
        self.motor_c_in1.value(1)
        self.motor_c_in2.value(0)
        self.motor_d_in1.value(1)
        self.motor_d_in2.value(0)
        
        self._update_command_time()
        return "left"
    
    def turn_right(self):
        """Tank-style right turn (right motors backward, left motors forward)"""
        # Left side motors - forward
        self.motor_a_in1.value(1)
        self.motor_a_in2.value(0)
        self.motor_b_in1.value(1)
        self.motor_b_in2.value(0)
        
        # Right side motors - backward
        self.motor_c_in1.value(0)
        self.motor_c_in2.value(1)
        self.motor_d_in1.value(0)
        self.motor_d_in2.value(1)
        
        self._update_command_time()
        return "right"
    
    def stop(self):
        """Stop all motors"""
        # Left side motors
        self.motor_a_in1.value(0)
        self.motor_a_in2.value(0)
        self.motor_b_in1.value(0)
        self.motor_b_in2.value(0)
        
        # Right side motors
        self.motor_c_in1.value(0)
        self.motor_c_in2.value(0)
        self.motor_d_in1.value(0)
        self.motor_d_in2.value(0)
        
        self._update_command_time()
        return "stop"
    
    def emergency_stop(self):
        """Emergency stop - immediately stop all motors"""
        self.stop()
        print("EMERGENCY STOP ACTIVATED")
        return "emergency_stop"
    
    def check_timeout(self):
        """Check if motors should be stopped due to command timeout"""
        if time.time() - self.last_command_time > self.timeout_duration:
            self.stop()
            return True
        return False