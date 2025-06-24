# ESP32 Smart Car Project

## Overview
This project implements a smartphone/tablet-controlled smart car using an ESP32 WROOM-32 C microcontroller. The car features tank-style movement with independent side control, a real-time web-based control interface, and various hardware control features.

## Features
- Full tank-style movement (forward/backward/left/right with independent side control)
- Real-time web-based control interface served directly from the ESP32
- Comprehensive lighting system (headlights, taillights, indicators, underglow)
- Horn/buzzer control
- Battery level monitoring with safety cutoff and warning alerts
- Fail-safe mechanisms (motor blocked, overcurrent, no signal timeout)
- Real-time motor speed feedback (PWM-aware)
- Web dashboard showing motor speed, active lights, horn status, battery voltage, and system status

## Hardware Requirements

### Core Components
- ESP32 WROOM-32 C (OceanLabz, CP2102, dual-core)
- L298N motor driver
- 4 DC gear motors (2-4V)
- Li-ion battery pack with BMS
- Chassis frame and wheels
- Various LEDs for lighting system
- Buzzer for horn
- Voltage divider for battery monitoring
- Power switch
- Jumper wires and connectors

See the [Parts List](parts_list.md) for a complete buying guide.

## Wiring Diagram

![Wiring Diagram](diagrams/wiring_diagram.png)

### Pin Connections

#### Motor Driver (L298N)
- ENA -> GPIO 25 (PWM)
- IN1 -> GPIO 26
- IN2 -> GPIO 27
- ENB -> GPIO 14 (PWM)
- IN3 -> GPIO 12
- IN4 -> GPIO 13
- 12V -> Battery Positive
- GND -> Battery Negative and ESP32 GND

#### Lighting System
- Headlights -> GPIO 4 (PWM)
- Taillights -> GPIO 16
- Left Indicator -> GPIO 17
- Right Indicator -> GPIO 18
- Underglow -> GPIO 19 (PWM)

#### Other Components
- Horn/Buzzer -> GPIO 5
- Battery Voltage Sensor -> GPIO 34 (ADC)

## Setup Guide

### Hardware Assembly
1. Assemble the chassis according to the manufacturer's instructions
2. Mount the motors to the chassis
3. Attach the wheels to the motors
4. Mount the ESP32 and L298N motor driver to the chassis
5. Connect the motors to the L298N driver
6. Wire the LEDs, buzzer, and battery voltage sensor according to the wiring diagram
7. Connect the battery to the system through a power switch

### Software Setup
1. Install the required development tools:
   - For MicroPython: Install Python, esptool, and ampy
   - For ESP-IDF/C++: Install ESP-IDF and related tools
2. Clone this repository
3. Upload the firmware to the ESP32 using the provided scripts
4. Connect to the ESP32's Wi-Fi access point (SSID: "ESP32-SmartCar", Password: "smartcar123")
5. Access the web interface at http://192.168.4.1

### Calibration
1. Motor Calibration:
   - Access the calibration page at http://192.168.4.1/calibration
   - Follow the on-screen instructions to calibrate the motors
   - Save the calibration settings

2. Battery Calibration:
   - Measure the actual battery voltage using a multimeter
   - Enter the measured voltage in the calibration page
   - Save the calibration settings

## OTA Updates

### Firmware Updates
1. Connect your computer to the same Wi-Fi network as the ESP32
2. Access the OTA update page at http://192.168.4.1/update
3. Select the new firmware file and click "Upload"
4. Wait for the update to complete and the ESP32 to restart

### Web Interface Updates
1. Access the OTA update page at http://192.168.4.1/update
2. Select the "Web Interface" tab
3. Select the new web interface files and click "Upload"
4. Wait for the update to complete

## User Experience Flow

1. Power on the smart car
2. Connect to the ESP32's Wi-Fi access point
3. Open the web interface in a browser
4. Use the on-screen controls to drive the car and control its features
5. Monitor the dashboard for system status and alerts
6. Power off the car when finished

## Project Structure

```
/
├── firmware/                 # ESP32 firmware code
│   ├── main/                 # Main application code
│   │   ├── main.cpp          # Entry point
│   │   ├── motor.cpp         # Motor control module
│   │   ├── motor.h           # Motor control header
│   │   ├── lights.cpp        # Lighting system module
│   │   ├── lights.h          # Lighting system header
│   │   ├── battery.cpp       # Battery monitoring module
│   │   ├── battery.h         # Battery monitoring header
│   │   ├── webserver.cpp     # Web server module
│   │   ├── webserver.h       # Web server header
│   │   ├── failsafe.cpp      # Fail-safe mechanisms module
│   │   ├── failsafe.h        # Fail-safe mechanisms header
│   │   └── config.h          # Configuration parameters
│   ├── components/           # External components
│   └── CMakeLists.txt        # CMake build file
├── web/                      # Web interface files
│   ├── index.html            # Main page
│   ├── css/                  # CSS styles
│   │   └── style.css         # Main stylesheet
│   ├── js/                   # JavaScript files
│   │   ├── main.js           # Main script
│   │   ├── controls.js       # Control interface
│   │   └── dashboard.js      # Dashboard interface
│   └── img/                  # Images
├── diagrams/                 # Project diagrams
│   └── wiring_diagram.png    # Wiring diagram
├── docs/                     # Documentation
│   ├── knowledge.md          # Technical knowledge base
│   └── api.md                # API documentation
├── scripts/                  # Utility scripts
│   ├── upload.sh             # Firmware upload script
│   └── ota_update.py         # OTA update script
├── parts_list.md             # Complete parts buying list
└── README.md                 # This file
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- ESP32 Community for their excellent documentation and examples
- Contributors to the open-source libraries used in this project