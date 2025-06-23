# Smart Rover - ESP8266 Wi-Fi Controlled Robot

A fully-featured, smartphone/tablet-controlled robotic car using an ESP8266 (flashed with MicroPython), an L298N motor driver, and 4 gear motors for tank-style movement.

## Features

- **Wi-Fi Control**: Connect directly to the rover's access point from any device with a web browser
- **Tank-Style Movement**: Forward, backward, left, right with differential steering
- **Variable Speed Control**: Adjust motor speed on the fly
- **Lighting System**: Headlights, taillights, turn indicators, and hazard lights
- **Special Effects**: Police lights, Knight Rider scanner effect
- **Horn/Buzzer**: Sound alerts with adjustable tones
- **Safety Features**: Auto-stop on signal loss or disconnection
- **Responsive Web Interface**: Works on smartphones, tablets, and computers
- **Real-time Telemetry**: Connection status, motor direction, and more

## Hardware Requirements

- ESP8266 NodeMCU or similar (flashed with MicroPython)
- L298N Motor Driver Module
- 4 DC Gear Motors (tank-style configuration)
- LEDs for lights (2 front, 2 rear, 2 indicators)
- Buzzer/Piezo Speaker
- Power Supply (Battery pack, recommended 6-12V)
- Jumper Wires
- Chassis/Frame for mounting components

## Pin Connections

### Motor Driver (L298N)

| ESP8266 Pin | L298N Pin | Function |
|-------------|-----------|----------|
| D1 (GPIO5)  | ENA       | Left Front Motor Speed |
| D2 (GPIO4)  | IN1       | Left Front Motor Direction 1 |
| D3 (GPIO0)  | IN2       | Left Front Motor Direction 2 |
| D4 (GPIO2)  | ENB       | Left Rear Motor Speed |
| D5 (GPIO14) | IN3       | Left Rear Motor Direction 1 |
| D6 (GPIO12) | IN4       | Left Rear Motor Direction 2 |
| D7 (GPIO13) | ENC       | Right Front Motor Speed |
| D8 (GPIO15) | IN5       | Right Front Motor Direction 1 |
| RX (GPIO3)  | IN6       | Right Front Motor Direction 2 |
| TX (GPIO1)  | END       | Right Rear Motor Speed |
| D0 (GPIO16) | IN7       | Right Rear Motor Direction 1 |
| SD3 (GPIO10)| IN8       | Right Rear Motor Direction 2 |

### Lights and Buzzer

| ESP8266 Pin | Component | Function |
|-------------|-----------|----------|
| SD2 (GPIO9) | LED       | Left Headlight (PWM) |
| SD1 (GPIO11)| LED       | Right Headlight (PWM) |
| SD0 (GPIO8) | LED       | Left Taillight |
| D10 (GPIO7) | LED       | Right Taillight |
| D9 (GPIO6)  | LED       | Left Indicator |
| D1 (GPIO5)  | LED       | Right Indicator |
| D2 (GPIO4)  | Buzzer    | Horn/Buzzer (PWM) |

## Wiring Diagram

```
+---------------------+                 +-------------------+
|                     |                 |                   |
|     ESP8266         |                 |     L298N         |
|                     |                 |     Driver        |
|  D1 (GPIO5)  -------|---------------->| ENA              |
|  D2 (GPIO4)  -------|---------------->| IN1              |
|  D3 (GPIO0)  -------|---------------->| IN2              |
|  D4 (GPIO2)  -------|---------------->| ENB              |
|  D5 (GPIO14) -------|---------------->| IN3              |
|  D6 (GPIO12) -------|---------------->| IN4              |
|  D7 (GPIO13) -------|---------------->| ENC              |
|  D8 (GPIO15) -------|---------------->| IN5              |
|  RX (GPIO3)  -------|---------------->| IN6              |
|  TX (GPIO1)  -------|---------------->| END              |
|  D0 (GPIO16) -------|---------------->| IN7              |
|  SD3 (GPIO10)-------|---------------->| IN8              |
|                     |                 |                   |
+---------------------+                 +-------------------+
                                         |       |       |
                                         v       v       v
                                        +--+    +--+    +--+
                                        |M1|    |M2|    |M3|
                                        +--+    +--+    +--+

+---------------------+
|                     |
|     ESP8266         |
|                     |
|  SD2 (GPIO9)  ------|----[330Ω]---->(Left Headlight)
|  SD1 (GPIO11) ------|----[330Ω]---->(Right Headlight)
|  SD0 (GPIO8)  ------|----[330Ω]---->(Left Taillight)
|  D10 (GPIO7)  ------|----[330Ω]---->(Right Taillight)
|  D9 (GPIO6)   ------|----[330Ω]---->(Left Indicator)
|  D1 (GPIO5)   ------|----[330Ω]---->(Right Indicator)
|  D2 (GPIO4)   ------|----[Buzzer]-->(Horn)
|                     |
+---------------------+
```

## Software Setup

### Prerequisites

1. Flash MicroPython firmware onto your ESP8266
2. Install a tool for uploading files to the ESP8266 (e.g., ampy, rshell, or WebREPL)

### Installation

1. Clone this repository or download the files
2. Upload all the Python files to your ESP8266:
   - `boot.py`
   - `main.py`
   - `motor_controller.py`
   - `light_system.py`
   - `safety_monitor.py`
   - `web_server.py`
   - `index.html`

3. Reset the ESP8266 to start the Smart Rover

### Configuration

You can modify the following settings in `main.py`:

```python
# Configuration
WIFI_SSID = "SmartRover"  # Change to your preferred AP name
WIFI_PASSWORD = "roverpwd123"  # Change to your preferred password
WEB_PORT = 80  # Web server port
```

## Usage

1. Power on the Smart Rover
2. Connect to the "SmartRover" Wi-Fi network with password "roverpwd123" (or your custom settings)
3. Open a web browser and navigate to `http://192.168.4.1`
4. Use the web interface to control the rover

### Control Interface

The web interface provides the following controls:

- **Movement Controls**: Forward, backward, left, right, and stop buttons
- **Speed Control**: Slider to adjust motor speed
- **Light Controls**: Toggle buttons for headlights, taillights, indicators, and hazard lights
- **Horn Button**: Press and hold for sound
- **Special Effects**: Buttons for police lights and Knight Rider scanner effect
- **Telemetry**: Real-time status information

## Safety Features

- **Auto-Stop**: Motors automatically stop if no commands are received for 2 seconds
- **Connection Monitoring**: Activates hazard lights and stops motors if connection is lost
- **Emergency Stop**: Immediately stops all motors

## Customization

### Adding New Features

The modular design makes it easy to add new features:

1. Add new functionality to the appropriate module
2. Update the web interface in `index.html`
3. Add command handlers in `web_server.py`

### Modifying Light Patterns

You can create custom light patterns by adding new methods to the `LightSystem` class in `light_system.py`.

## Troubleshooting

- **Connection Issues**: Ensure you're connected to the correct Wi-Fi network
- **Motors Not Responding**: Check wiring and battery level
- **WebREPL Access**: Connect to the rover's Wi-Fi and access WebREPL at `http://192.168.4.1:8266`

## License

This project is open source and available under the MIT License.

## Acknowledgments

- MicroPython community for the excellent ESP8266 support
- Various open-source robotics projects that inspired this design