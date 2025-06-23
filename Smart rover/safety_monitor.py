# Safety Monitor Module for Smart Rover
# Implements safety features like auto-stop on signal loss

import time

class SafetyMonitor:
    def __init__(self, motor_controller, light_system):
        self.motor_controller = motor_controller
        self.light_system = light_system
        
        # Safety parameters
        self.last_heartbeat = time.time()
        self.heartbeat_timeout = 5  # seconds
        self.is_emergency_mode = False
        self.connection_active = False
        
        # System status
        self.system_status = {
            "connection": "initializing",
            "motors": "stopped",
            "battery": "unknown",
            "last_command": "",
            "uptime": 0,
            "start_time": time.time()
        }
        
        print("Safety Monitor initialized")
    
    def update_heartbeat(self):
        """Update the heartbeat timestamp when receiving commands"""
        self.last_heartbeat = time.time()
        self.connection_active = True
        self.system_status["connection"] = "connected"
        
        # If recovering from emergency mode
        if self.is_emergency_mode:
            self.is_emergency_mode = False
            print("Connection restored, exiting emergency mode")
    
    def check(self):
        """Perform safety checks - call this regularly from main loop"""
        current_time = time.time()
        
        # Update uptime
        self.system_status["uptime"] = int(current_time - self.system_status["start_time"])
        
        # Check for connection timeout
        if self.connection_active and current_time - self.last_heartbeat > self.heartbeat_timeout:
            self._handle_connection_loss()
        
        # Check motor timeout (separate from connection timeout)
        motor_timeout = self.motor_controller.check_timeout()
        if motor_timeout:
            self.system_status["motors"] = "timeout_stopped"
        
        return self.system_status
    
    def _handle_connection_loss(self):
        """Handle connection loss event"""
        print("WARNING: Connection lost! Activating emergency mode")
        
        # Update status
        self.connection_active = False
        self.is_emergency_mode = True
        self.system_status["connection"] = "disconnected"
        
        # Stop motors
        self.motor_controller.emergency_stop()
        self.system_status["motors"] = "emergency_stopped"
        
        # Activate hazard lights
        self.light_system.hazard_lights(True)
    
    def get_status(self):
        """Get current system status"""
        return self.system_status
    
    def update_status(self, key, value):
        """Update a specific status value"""
        if key in self.system_status:
            self.system_status[key] = value
            return True
        return False