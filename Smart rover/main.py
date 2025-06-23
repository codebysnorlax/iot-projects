# Smart Rover - Main Controller
# MicroPython implementation for ESP8266

import network
import socket
import time
from machine import Pin, PWM
import json

# Import our custom modules
from motor_controller import MotorController
from light_system import LightSystem
from safety_monitor import SafetyMonitor
from web_server import WebServer

# Configuration
WIFI_SSID = "SmartRover"
WIFI_PASSWORD = "roverpwd123"
WEB_PORT = 80

# Initialize components
motor_controller = None
light_system = None
safety_monitor = None
web_server = None

def setup():
    global motor_controller, light_system, safety_monitor, web_server
    
    print("Initializing Smart Rover...")
    
    # Setup WiFi Access Point
    ap = network.WLAN(network.AP_IF)
    ap.active(True)
    ap.config(essid=WIFI_SSID, password=WIFI_PASSWORD)
    while not ap.active():
        pass
    print(f"WiFi AP active. Connect to {WIFI_SSID} with password {WIFI_PASSWORD}")
    print(f"Rover IP address: {ap.ifconfig()[0]}")
    
    # Initialize components
    motor_controller = MotorController()
    light_system = LightSystem()
    safety_monitor = SafetyMonitor(motor_controller, light_system)
    
    # Initialize web server with references to components
    web_server = WebServer(motor_controller, light_system, safety_monitor)
    web_server.start()
    
    print("Smart Rover initialization complete!")

def main_loop():
    try:
        while True:
            # Process safety checks
            safety_monitor.check()
            
            # Give control to the web server
            web_server.process()
            
            # Small delay to prevent CPU hogging
            time.sleep(0.01)
    except Exception as e:
        print(f"Error in main loop: {e}")
        # Emergency stop if there's an error
        if motor_controller:
            motor_controller.emergency_stop()
        # Blink lights to indicate error
        if light_system:
            light_system.error_mode()

if __name__ == "__main__":
    setup()
    main_loop()