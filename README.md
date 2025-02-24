# ESP8266 Servo Control Web Server

## Overview
This project allows you to control a servo motor connected to an ESP8266/ESP32 through a web-based interface. The ESP8266 runs a web server, which serves an HTML page with a slider to adjust the servo angle. The slider value is sent to the ESP8266 via HTTP requests, and the servo moves accordingly.

## Features
- Connects ESP8266 to a Wi-Fi network.
- Runs a web server to serve an HTML control page.
- Allows real-time control of a servo motor via a web-based slider.

## Hardware Requirements
- ESP8266 or ESP32
- SG90 Servo Motor
- Jumper Wires
- Power Supply (USB or external 5V source)

## Software Requirements
- MicroPython installed on ESP8266/ESP32
- A tool to upload files (like `ampy` or `WebREPL`)

## Setup Instructions
1. **Flash MicroPython** onto your ESP8266/ESP32 if not already installed.
2. **Upload Files**:
   - `main.py` (handles Wi-Fi connection and server logic)
   - `index.html` (web interface for controlling the servo)
3. **Edit Wi-Fi Credentials** in `main.py`:
   ```python
   SSID = "your_ssid"   # Your Wi-Fi SSID
   PASSWORD = "your_password"  # Your Wi-Fi Password
   ```
4. **Restart ESP8266** and access the web interface using the IP address printed in the serial monitor.

## Code Explanation
### `main.py` - Connecting to Wi-Fi
This script connects the ESP8266 to a Wi-Fi network. If unsuccessful, it retries for 10 seconds before failing.
```python
import network
import time

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    
    if not wlan.isconnected():
        print("Connecting to Wi-Fi...")
        wlan.connect(SSID, PASSWORD)

        timeout = 10
        while not wlan.isconnected() and timeout > 0:
            time.sleep(1)
            timeout -= 1
            print("Retrying...")

    if wlan.isconnected():
        print("Connected! IP Address:", wlan.ifconfig()[0])
    else:
        print("Failed to connect. Check SSID/Password.")
```

### `main.py` - Web Server & Servo Control
This script sets up a web server on ESP8266 and controls a servo motor based on HTTP requests.
```python
import socket
import machine

# Initialize Servo on GPIO2 (D4)
servo = machine.PWM(machine.Pin(2), freq=50)

def set_angle(angle):
    duty = int((angle / 180) * 75) + 40  # Convert angle to PWM duty cycle
    servo.duty(duty)

# Start Web Server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', 80))
s.listen(5)

print("Web server running...")

while True:
    conn, addr = s.accept()
    request = conn.recv(1024).decode()
    
    if "GET /servo?angle=" in request:
        try:
            angle = int(request.split("GET /servo?angle=")[1].split(" ")[0])
            set_angle(angle)
            print("Servo angle set to:", angle)
            response = "HTTP/1.1 200 OK\n\nOK"
        except:
            response = "HTTP/1.1 400 Bad Request\n\nInvalid Angle"
    else:
        response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + read_html()
    
    conn.send(response)
    conn.close()
```

### `index.html` - Web Interface
This HTML page provides a slider to control the servo's angle in real-time.
```html
<input type="range" id="slider" min="0" max="180" value="90" oninput="updateServo(this.value)">
<p>Angle: <span id="angleValue">90</span>°</p>

<script>
    function updateServo(angle) {
        document.getElementById("angleValue").innerText = angle;
        fetch(`/servo?angle=${angle}`);
    }
</script>
```

## How to Use
1. Power on your ESP8266/ESP32.
2. Find the IP address printed in the serial monitor.
3. Open the IP address in a web browser.
4. Adjust the slider to control the servo motor.

## License
This project is open-source and can be used freely for educational purposes.

---
Enjoy coding with ESP8266! 🚀

