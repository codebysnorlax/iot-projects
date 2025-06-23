# Light System Module for Smart Rover
# Handles headlights, taillights, indicators, and special light patterns

from machine import Pin, PWM
import time

class LightSystem:
    def __init__(self):
        # Pin Configuration for Lights
        # Headlights (Front LEDs)
        self.headlight_left = PWM(Pin(9))  # SD2 - PWM for brightness control
        self.headlight_right = PWM(Pin(11))  # SD1 - PWM for brightness control
        
        # Taillights (Back LEDs)
        self.taillight_left = Pin(8, Pin.OUT)  # SD0
        self.taillight_right = Pin(7, Pin.OUT)  # D10
        
        # Turn Indicators
        self.indicator_left = Pin(6, Pin.OUT)  # D9
        self.indicator_right = Pin(5, Pin.OUT)  # D1
        
        # Buzzer/Horn
        self.buzzer = PWM(Pin(4))  # D2 - PWM for tone control
        
        # Initialize PWM for headlights (0-1023)
        self.headlight_left.freq(1000)
        self.headlight_right.freq(1000)
        self.buzzer.freq(2000)  # Initial buzzer frequency
        
        # Set initial brightness to 0 (off)
        self.headlight_left.duty(0)
        self.headlight_right.duty(0)
        self.buzzer.duty(0)  # Buzzer off
        
        # Turn off all other lights
        self.taillight_left.value(0)
        self.taillight_right.value(0)
        self.indicator_left.value(0)
        self.indicator_right.value(0)
        
        # State tracking
        self.headlights_on = False
        self.taillights_on = False
        self.left_indicator_on = False
        self.right_indicator_on = False
        self.hazard_lights_on = False
        self.horn_on = False
        
        # For blinking effects
        self.last_blink_time = time.time()
        self.blink_state = False
        self.blink_interval = 0.5  # seconds
        
        print("Light System initialized")
    
    def headlights(self, state, brightness=100):
        """Control headlights with optional brightness (0-100%)"""
        duty = int(brightness * 10.23)  # Convert 0-100 to 0-1023
        if state:
            # Fade in effect
            current = 0
            while current < duty:
                self.headlight_left.duty(current)
                self.headlight_right.duty(current)
                current += 50
                time.sleep(0.01)
            # Ensure final brightness is exactly as requested
            self.headlight_left.duty(duty)
            self.headlight_right.duty(duty)
        else:
            # Immediate off
            self.headlight_left.duty(0)
            self.headlight_right.duty(0)
        
        self.headlights_on = state
        return "headlights_" + ("on" if state else "off")
    
    def taillights(self, state):
        """Control taillights"""
        self.taillight_left.value(1 if state else 0)
        self.taillight_right.value(1 if state else 0)
        self.taillights_on = state
        return "taillights_" + ("on" if state else "off")
    
    def brake_lights(self, state):
        """Control brake lights (taillights at full brightness)"""
        # For simple LEDs, this is the same as taillights
        # For PWM-controlled LEDs, this would set maximum brightness
        self.taillight_left.value(1 if state else 0)
        self.taillight_right.value(1 if state else 0)
        return "brake_lights_" + ("on" if state else "off")
    
    def left_indicator(self, state):
        """Control left turn indicator"""
        if state:
            self.left_indicator_on = True
            self.right_indicator_on = False
        else:
            self.left_indicator_on = False
            self.indicator_left.value(0)  # Immediately turn off
        return "left_indicator_" + ("on" if state else "off")
    
    def right_indicator(self, state):
        """Control right turn indicator"""
        if state:
            self.right_indicator_on = True
            self.left_indicator_on = False
        else:
            self.right_indicator_on = False
            self.indicator_right.value(0)  # Immediately turn off
        return "right_indicator_" + ("on" if state else "off")
    
    def hazard_lights(self, state):
        """Control hazard lights (both indicators blinking)"""
        if state:
            self.hazard_lights_on = True
            self.left_indicator_on = False
            self.right_indicator_on = False
        else:
            self.hazard_lights_on = False
            self.indicator_left.value(0)  # Immediately turn off
            self.indicator_right.value(0)  # Immediately turn off
        return "hazard_lights_" + ("on" if state else "off")
    
    def horn(self, state, tone=2000):
        """Control horn/buzzer with optional tone frequency"""
        self.buzzer.freq(tone)  # Set frequency
        if state:
            self.buzzer.duty(512)  # 50% duty cycle for buzzer
            self.horn_on = True
        else:
            self.buzzer.duty(0)  # Turn off
            self.horn_on = False
        return "horn_" + ("on" if state else "off")
    
    def police_lights(self, duration=5):
        """Special pattern: Police-like alternating lights for specified duration"""
        start_time = time.time()
        while time.time() - start_time < duration:
            # Left side blue (simulated with regular LED)
            self.indicator_left.value(1)
            self.indicator_right.value(0)
            time.sleep(0.1)
            
            # Right side red (simulated with regular LED)
            self.indicator_left.value(0)
            self.indicator_right.value(1)
            time.sleep(0.1)
        
        # Turn off all indicators
        self.indicator_left.value(0)
        self.indicator_right.value(0)
        return "police_lights_complete"
    
    def knight_rider(self, duration=5):
        """Special pattern: Knight Rider/KITT scanner effect"""
        start_time = time.time()
        while time.time() - start_time < duration:
            # Sequence: left to right then right to left
            for pin in [self.indicator_left, self.taillight_left, 
                       self.taillight_right, self.indicator_right]:
                pin.value(1)
                time.sleep(0.1)
                pin.value(0)
            
            for pin in [self.taillight_right, self.taillight_left, 
                       self.indicator_left]:
                pin.value(1)
                time.sleep(0.1)
                pin.value(0)
        return "knight_rider_complete"
    
    def update(self):
        """Update blinking lights - call this regularly from main loop"""
        current_time = time.time()
        
        # Check if it's time to toggle blink state
        if current_time - self.last_blink_time >= self.blink_interval:
            self.blink_state = not self.blink_state
            self.last_blink_time = current_time
            
            # Update indicators based on blink state
            if self.left_indicator_on:
                self.indicator_left.value(1 if self.blink_state else 0)
            
            if self.right_indicator_on:
                self.indicator_right.value(1 if self.blink_state else 0)
            
            if self.hazard_lights_on:
                self.indicator_left.value(1 if self.blink_state else 0)
                self.indicator_right.value(1 if self.blink_state else 0)
    
    def all_off(self):
        """Turn off all lights"""
        self.headlights(False)
        self.taillights(False)
        self.left_indicator(False)
        self.right_indicator(False)
        self.hazard_lights(False)
        self.horn(False)
        return "all_lights_off"
    
    def error_mode(self):
        """Error indication - rapid flashing of all lights"""
        for _ in range(10):  # Flash 10 times
            # All lights on
            self.headlight_left.duty(1023)
            self.headlight_right.duty(1023)
            self.taillight_left.value(1)
            self.taillight_right.value(1)
            self.indicator_left.value(1)
            self.indicator_right.value(1)
            self.buzzer.duty(512)  # Buzzer on
            time.sleep(0.1)
            
            # All lights off
            self.headlight_left.duty(0)
            self.headlight_right.duty(0)
            self.taillight_left.value(0)
            self.taillight_right.value(0)
            self.indicator_left.value(0)
            self.indicator_right.value(0)
            self.buzzer.duty(0)  # Buzzer off
            time.sleep(0.1)
        
        # Reset all states
        self.all_off()
        return "error_mode_complete"