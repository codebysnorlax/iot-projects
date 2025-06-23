# Web Server Module for Smart Rover
# Handles Wi-Fi communication and web interface

import socket
import json
import time

class WebServer:
    def __init__(self, motor_controller, light_system, safety_monitor):
        self.motor_controller = motor_controller
        self.light_system = light_system
        self.safety_monitor = safety_monitor
        
        # Server configuration
        self.port = 80
        self.socket = None
        self.clients = []
        self.max_clients = 5
        
        # Command mapping
        self.command_handlers = {
            # Motor commands
            "forward": self._handle_forward,
            "backward": self._handle_backward,
            "left": self._handle_left,
            "right": self._handle_right,
            "stop": self._handle_stop,
            "speed": self._handle_speed,
            
            # Light commands
            "headlights": self._handle_headlights,
            "taillights": self._handle_taillights,
            "left_indicator": self._handle_left_indicator,
            "right_indicator": self._handle_right_indicator,
            "hazard": self._handle_hazard,
            "horn": self._handle_horn,
            "light_pattern": self._handle_light_pattern,
            
            # System commands
            "status": self._handle_status,
            "heartbeat": self._handle_heartbeat
        }
        
        print("Web Server initialized")
    
    def start(self):
        """Start the web server"""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        # Bind to all interfaces
        self.socket.bind(('0.0.0.0', self.port))
        self.socket.listen(5)
        self.socket.setblocking(False)
        
        print(f"Web server started on port {self.port}")
        return True
    
    def process(self):
        """Process incoming connections and client data"""
        try:
            # Accept new connections
            self._accept_new_clients()
            
            # Process existing clients
            self._process_clients()
            
            # Update light system (for blinking effects)
            self.light_system.update()
            
        except Exception as e:
            print(f"Error in web server processing: {e}")
    
    def _accept_new_clients(self):
        """Accept new client connections"""
        try:
            client_socket, addr = self.socket.accept()
            print(f"New client connected: {addr}")
            client_socket.setblocking(False)
            
            # Send the HTML interface to the client
            self._send_web_interface(client_socket)
            
            # Add to clients list if not full
            if len(self.clients) < self.max_clients:
                self.clients.append(client_socket)
            else:
                # Too many clients, reject with message
                client_socket.send("HTTP/1.1 503 Service Unavailable\r\nContent-Type: text/plain\r\n\r\nToo many connections".encode())
                client_socket.close()
        except OSError as e:
            # No new connections available (non-blocking socket)
            pass
    
    def _process_clients(self):
        """Process data from connected clients"""
        for client in self.clients.copy():  # Use copy to safely remove during iteration
            try:
                data = client.recv(1024)
                if data:
                    self._handle_client_data(client, data)
                else:
                    # Empty data means client disconnected
                    self.clients.remove(client)
                    client.close()
            except OSError as e:
                # No data available or client disconnected
                pass
    
    def _handle_client_data(self, client, data):
        """Handle data received from a client"""
        try:
            # Check if it's an HTTP request
            request = data.decode('utf-8')
            
            # Handle API requests
            if "GET /api/" in request:
                self._handle_api_request(client, request)
            # Handle WebSocket data (if implemented)
            elif "\r\n\r\n" in request and len(request.split("\r\n\r\n")[1]) > 0:
                payload = request.split("\r\n\r\n")[1]
                self._handle_websocket_data(client, payload)
        except Exception as e:
            print(f"Error handling client data: {e}")
    
    def _handle_api_request(self, client, request):
        """Handle API requests from HTTP GET"""
        # Extract the API endpoint and parameters
        request_line = request.split('\r\n')[0]
        path = request_line.split(' ')[1]
        
        # Remove /api/ prefix
        if path.startswith('/api/'):
            endpoint = path[5:]
            
            # Split endpoint and parameters
            parts = endpoint.split('/')
            command = parts[0]
            params = parts[1:] if len(parts) > 1 else []
            
            # Execute the command
            response = self._execute_command(command, params)
            
            # Send response
            http_response = f"HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{json.dumps(response)}"
            client.send(http_response.encode())
        else:
            # Not an API request, send 404
            client.send("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found".encode())
    
    def _execute_command(self, command, params):
        """Execute a command with parameters"""
        # Update heartbeat for any command
        self.safety_monitor.update_heartbeat()
        
        # Find and execute the command handler
        if command in self.command_handlers:
            handler = self.command_handlers[command]
            result = handler(params)
            
            # Update last command in status
            self.safety_monitor.update_status("last_command", command)
            
            return {
                "status": "success",
                "command": command,
                "result": result
            }
        else:
            return {
                "status": "error",
                "message": f"Unknown command: {command}"
            }
    
    # Command Handlers
    def _handle_forward(self, params):
        result = self.motor_controller.forward()
        self.light_system.taillights(False)  # Turn off brake lights
        return result
    
    def _handle_backward(self, params):
        result = self.motor_controller.backward()
        self.light_system.taillights(True)  # Turn on brake/reverse lights
        return result
    
    def _handle_left(self, params):
        result = self.motor_controller.turn_left()
        self.light_system.left_indicator(True)  # Turn on left indicator
        self.light_system.right_indicator(False)  # Ensure right indicator is off
        return result
    
    def _handle_right(self, params):
        result = self.motor_controller.turn_right()
        self.light_system.right_indicator(True)  # Turn on right indicator
        self.light_system.left_indicator(False)  # Ensure left indicator is off
        return result
    
    def _handle_stop(self, params):
        result = self.motor_controller.stop()
        self.light_system.taillights(True)  # Turn on brake lights
        self.light_system.left_indicator(False)  # Turn off indicators
        self.light_system.right_indicator(False)
        return result
    
    def _handle_speed(self, params):
        if params and params[0].isdigit():
            speed = int(params[0])
            return self.motor_controller.set_speed(speed)
        return "invalid_speed"
    
    def _handle_headlights(self, params):
        if params:
            state = params[0].lower() == "on" or params[0].lower() == "true" or params[0] == "1"
            brightness = int(params[1]) if len(params) > 1 and params[1].isdigit() else 100
            return self.light_system.headlights(state, brightness)
        return "invalid_headlight_params"
    
    def _handle_taillights(self, params):
        if params:
            state = params[0].lower() == "on" or params[0].lower() == "true" or params[0] == "1"
            return self.light_system.taillights(state)
        return "invalid_taillight_params"
    
    def _handle_left_indicator(self, params):
        if params:
            state = params[0].lower() == "on" or params[0].lower() == "true" or params[0] == "1"
            return self.light_system.left_indicator(state)
        return "invalid_left_indicator_params"
    
    def _handle_right_indicator(self, params):
        if params:
            state = params[0].lower() == "on" or params[0].lower() == "true" or params[0] == "1"
            return self.light_system.right_indicator(state)
        return "invalid_right_indicator_params"
    
    def _handle_hazard(self, params):
        if params:
            state = params[0].lower() == "on" or params[0].lower() == "true" or params[0] == "1"
            return self.light_system.hazard_lights(state)
        return "invalid_hazard_params"
    
    def _handle_horn(self, params):
        if params:
            state = params[0].lower() == "on" or params[0].lower() == "true" or params[0] == "1"
            tone = int(params[1]) if len(params) > 1 and params[1].isdigit() else 2000
            return self.light_system.horn(state, tone)
        return "invalid_horn_params"
    
    def _handle_light_pattern(self, params):
        if params:
            pattern = params[0].lower()
            duration = int(params[1]) if len(params) > 1 and params[1].isdigit() else 5
            
            if pattern == "police":
                return self.light_system.police_lights(duration)
            elif pattern == "knight_rider" or pattern == "kitt":
                return self.light_system.knight_rider(duration)
            else:
                return "unknown_pattern"
        return "invalid_pattern_params"
    
    def _handle_status(self, params):
        return self.safety_monitor.get_status()
    
    def _handle_heartbeat(self, params):
        self.safety_monitor.update_heartbeat()
        return "heartbeat_acknowledged"
    
    def _send_web_interface(self, client):
        """Send the HTML web interface to the client"""
        with open('index.html', 'r') as file:
            content = file.read()
            
            response = "HTTP/1.1 200 OK\r\n"
            response += "Content-Type: text/html\r\n"
            response += f"Content-Length: {len(content)}\r\n"
            response += "Connection: keep-alive\r\n\r\n"
            response += content
            
            client.send(response.encode())