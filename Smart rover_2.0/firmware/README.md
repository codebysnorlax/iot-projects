# ESP32 Smart Car Project

## Overview

This project implements a smartphone/tablet-controlled smart car using an ESP32 WROOM-32 C microcontroller, L298N motor driver, and 4 DC gear motors. The car features tank-style movement with independent side control, a real-time web-based control interface served directly from the ESP32, and a rich set of hardware features.

### Key Features

- **Tank-style Movement**: Full forward/backward/left/right control with independent side management
- **Real-time Web Interface**: Mobile-friendly UI served directly from the ESP32
- **Comprehensive Lighting System**: Headlights, taillights, indicators, and underglow LEDs
- **Horn/Buzzer Control**: Sound alerts on demand
- **Battery Monitoring**: Real-time voltage tracking with safety cutoff and alerts
- **Failsafe Mechanisms**: Protection against motor blocking, overcurrent, signal loss
- **Real-time Feedback**: Motor speed, light status, battery level, system status
- **OTA Updates**: Over-the-air firmware and web interface updates
- **Dual-core Optimization**: Task distribution across both ESP32 cores

## Hardware Requirements

### Core Components

- ESP32 WROOM-32 C development board (OceanLabz, CP2102, dual-core)
- L298N motor driver module
- 4× DC gear motors (2-4V)
- 18650 Li-ion batteries (2×) with battery holder
- Battery Management System (BMS) with protection circuit
- Car chassis with wheels

### Additional Components

- LEDs for lighting system (white, red, amber)
- WS2812B addressable RGB LED strip for underglow
- Passive buzzer for horn
- Voltage divider resistors (10kΩ and 2.2kΩ)
- Power switch
- Jumper wires
- Breadboard or PCB for connections

See the [Complete Parts List](#complete-parts-list) section for detailed purchasing information.

## Wiring Diagram

```
+----------------+     +----------------+     +----------------+
|                |     |                |     |                |
|    ESP32       |     |    L298N       |     |   Motors       |
|  WROOM-32 C    |     |  Motor Driver  |     |  (4× DC Gear)  |
|                |     |                |     |                |
+-------+--------+     +--------+-------+     +----------------+
        |                       |
        +-----------------------+
        |
+-------+--------+     +----------------+     +----------------+
|                |     |                |     |                |
|  Lights &      |     |   Battery &    |     |   Power        |
|  Indicators    |     |   Monitoring   |     |   Management   |
|                |     |                |     |                |
+----------------+     +----------------+     +----------------+
```

### Pin Connections

#### ESP32 to L298N Motor Driver

| ESP32 Pin | L298N Pin | Function |
|-----------|-----------|----------|
| GPIO 26   | IN1       | Left Motors Forward |
| GPIO 25   | IN2       | Left Motors Backward |
| GPIO 33   | IN3       | Right Motors Forward |
| GPIO 32   | IN4       | Right Motors Backward |
| GPIO 14   | ENA       | Left Motors Enable (PWM) |
| GPIO 27   | ENB       | Right Motors Enable (PWM) |
| GND       | GND       | Ground |

#### ESP32 to Lighting System

| ESP32 Pin | Component | Function |
|-----------|-----------|----------|
| GPIO 4    | LED + Resistor | Left Headlight |
| GPIO 16   | LED + Resistor | Right Headlight |
| GPIO 17   | PWM Control | Headlight Brightness |
| GPIO 18   | LED + Resistor | Left Taillight |
| GPIO 19   | LED + Resistor | Right Taillight |
| GPIO 21   | LED + Resistor | Left Indicator |
| GPIO 22   | LED + Resistor | Right Indicator |
| GPIO 23   | WS2812B Data | Underglow LEDs |

#### ESP32 to Other Components

| ESP32 Pin | Component | Function |
|-----------|-----------|----------|
| GPIO 5    | Passive Buzzer | Horn |
| GPIO 34   | Voltage Divider | Battery Monitoring |

#### Power Connections

| Component | Power Source | Notes |
|-----------|-------------|-------|
| ESP32     | 5V from BMS | Via USB or VIN pin |
| L298N     | 7.4V from Battery | Direct connection |
| Motors    | Via L298N   | Controlled by driver |
| LEDs      | 3.3V from ESP32 | Via current-limiting resistors |
| WS2812B   | 5V from BMS | Separate power connection |

## Software Setup

### Prerequisites

1. Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) (v4.4 or later)
2. Install [Visual Studio Code](https://code.visualstudio.com/) with the ESP-IDF extension (optional but recommended)
3. Clone this repository

### Building and Flashing

#### Using ESP-IDF Command Line

```bash
# Navigate to the project directory
cd firmware

# Configure the project
idf.py menuconfig

# Build the project
idf.py build

# Flash to ESP32
idf.py -p [PORT] flash

# Monitor serial output
idf.py -p [PORT] monitor
```

#### Using Visual Studio Code with ESP-IDF Extension

1. Open the project folder in VS Code
2. Configure the ESP-IDF extension with your ESP-IDF path
3. Use the ESP-IDF extension buttons to build, flash, and monitor

### First-time Setup

1. Flash the firmware to the ESP32
2. Power on the smart car
3. Connect to the Wi-Fi network "ESP32_SmartCar" (password: "smartcar123")
4. Open a web browser and navigate to http://192.168.4.1
5. Follow the calibration instructions on the web interface

## Calibration

### Motor Calibration

1. Place the car on a flat surface with wheels elevated
2. Navigate to the Calibration page from the web interface
3. Click "Start Full Calibration"
4. The system will run each motor individually and adjust parameters
5. When finished, click "Stop All Motors"

### Battery Calibration

1. Measure the actual battery voltage using a multimeter
2. Enter the measured voltage in the "Measured Battery Voltage" field
3. Click "Calibrate Battery"
4. The system will adjust the voltage divider ratio to match your measurement

## OTA Updates

### Firmware Updates

1. Build a new firmware binary using `idf.py build`
2. Navigate to the Update page on the web interface
3. Select the firmware tab
4. Upload the new firmware.bin file from the build/firmware.bin path
5. Wait for the update to complete and the device to restart

### Web Interface Updates

1. Compress the updated web interface files into a ZIP archive
2. Navigate to the Update page on the web interface
3. Select the web interface tab
4. Upload the ZIP file
5. Wait for the update to complete

## User Interface

The web interface provides the following features:

- **Dashboard**: Battery level, system status, motor speeds
- **Controls**: Virtual joystick for movement, buttons for lights and horn
- **Settings**: Calibration options, system information, OTA updates
- **Diagnostics**: System logs, component testing, troubleshooting
- **Help**: User guide, troubleshooting tips, FAQ

## Failsafe Mechanisms

The smart car includes several failsafe mechanisms to protect the hardware:

1. **Communication Timeout**: Stops motors if no commands are received for a set period
2. **Low Battery Protection**: Reduces functionality and eventually shuts down when battery is low
3. **Motor Blocking Detection**: Stops motors if resistance is detected to prevent damage
4. **Overcurrent Protection**: Monitors current draw and stops motors if excessive
5. **System Recovery**: Automatic recovery procedures for various failure conditions

## Complete Parts List

### Core Electronics

| Component | Specification | Quantity | Est. Price | Link/Alternative |
|-----------|--------------|----------|------------|------------------|
| ESP32 Development Board | WROOM-32 C, OceanLabz, CP2102, dual-core | 1 | $8-12 | [Amazon](https://www.amazon.com/ESP32-WROOM-32-Development-ESP-32S-Bluetooth-Arduino/dp/B084KWNMM4/) |
| L298N Motor Driver | Dual H-Bridge, 5-35V | 1 | $5-8 | [Amazon](https://www.amazon.com/HiLetgo-Controller-Stepper-H-Bridge-Mega2560/dp/B07BK1QL5T/) |
| DC Gear Motors | 3-6V, with wheels | 4 | $12-20 | [Amazon](https://www.amazon.com/DEYUE-Geared-Motor-200RPM-Arduino/dp/B07DQGX9N3/) |
| 18650 Li-ion Batteries | 3.7V, 2600mAh+ | 2 | $10-15 | [Amazon](https://www.amazon.com/EBL-Rechargeable-Batteries-2300mAh-Battery/dp/B08HN1VCBJ/) |
| 18650 Battery Holder | 2-slot with wires | 1 | $3-5 | [Amazon](https://www.amazon.com/Battery-Holder-Leads-Wires-Black/dp/B07TXDWKB3/) |
| Battery Protection Board | 2S BMS, 8.4V, with charging | 1 | $5-8 | [Amazon](https://www.amazon.com/Protection-Lithium-Battery-Balance-Function/dp/B07KSPWHGN/) |

### Chassis and Structure

| Component | Specification | Quantity | Est. Price | Link/Alternative |
|-----------|--------------|----------|------------|------------------|
| Car Chassis Kit | 4WD with motor mounts | 1 | $15-25 | [Amazon](https://www.amazon.com/perseids-Chassis-Encoder-Battery-Arduino/dp/B07DNYQ3P5/) |
| Jumper Wires | Male-to-male, male-to-female, female-to-female | 40+ | $6-10 | [Amazon](https://www.amazon.com/EDGELEC-Breadboard-Optional-Assorted-Multicolored/dp/B07GD2BWPY/) |
| Breadboard | Mini size, 170 tie points | 1 | $3-5 | [Amazon](https://www.amazon.com/DEYUE-breadboard-Set-Prototype-Board/dp/B07LFD4LT6/) |
| Power Switch | SPST toggle switch | 1 | $2-4 | [Amazon](https://www.amazon.com/ZUPAYIPA-Position-Toggle-Switch-Arduino/dp/B01N2U8PK0/) |

### Lighting and Indicators

| Component | Specification | Quantity | Est. Price | Link/Alternative |
|-----------|--------------|----------|------------|------------------|
| White LEDs | 5mm, high brightness | 2 | $3-5 | [Amazon](https://www.amazon.com/DiCUNO-450pcs-Colors-Emitting-Assorted/dp/B073QMYKDM/) |
| Red LEDs | 5mm | 2 | Included above | Included in LED pack |
| Amber LEDs | 5mm | 2 | Included above | Included in LED pack |
| WS2812B LED Strip | Addressable RGB, 30 LEDs/m | 0.5m | $8-12 | [Amazon](https://www.amazon.com/BTF-LIGHTING-Flexible-Individually-Addressable-Non-waterproof/dp/B01CDTEJBG/) |
| Resistors Kit | Various values (220Ω, 330Ω, 10kΩ, etc.) | 1 kit | $6-10 | [Amazon](https://www.amazon.com/ELEGOO-Values-Resistor-Assortment-Compliant/dp/B072BL2VX1/) |

### Additional Components

| Component | Specification | Quantity | Est. Price | Link/Alternative |
|-----------|--------------|----------|------------|------------------|
| Passive Buzzer | 5V | 1 | $5-8 | [Amazon](https://www.amazon.com/Cylewet-Electronic-Magnetic-Continuous-Arduino/dp/B01NCOXB2Q/) |
| USB Cable | USB-A to Micro USB | 1 | $3-6 | [Amazon](https://www.amazon.com/AmazonBasics-Male-Micro-Cable-Black/dp/B0711PVX6Z/) |
| Heat Shrink Tubing | Various sizes | 1 kit | $6-10 | [Amazon](https://www.amazon.com/560PCS-Heat-Shrink-Tubing-Eventronic/dp/B072PCQ2LW/) |

### Tools (if you don't already have them)

| Tool | Specification | Quantity | Est. Price | Link/Alternative |
|-----------|--------------|----------|------------|------------------|
| Soldering Iron Kit | 60W with accessories | 1 | $20-30 | [Amazon](https://www.amazon.com/Soldering-Iron-Kit-Temperature-Desoldering/dp/B07GTGGLXN/) |
| Digital Multimeter | Basic voltage/current/resistance | 1 | $15-25 | [Amazon](https://www.amazon.com/AstroAI-Digital-Multimeter-Voltage-Tester/dp/B01ISAMUA6/) |
| Wire Stripper/Cutter | 22-30 AWG | 1 | $8-12 | [Amazon](https://www.amazon.com/DOWELL-Stripper-Multi-Function-Tool%EF%BC%8CProfessional-Craftsmanship/dp/B06X9875Z7/) |
| Small Screwdriver Set | Phillips and flathead | 1 | $8-15 | [Amazon](https://www.amazon.com/Screwdriver-Precision-Smartphone-Eyeglasses-Electronics/dp/B07YJG766F/) |

### Budget Options and Alternatives

- **Motors**: Use 2 motors instead of 4 (with a 2WD chassis)
- **Battery**: Use 4 AA batteries with a holder instead of 18650 Li-ion
- **Chassis**: Build your own using cardboard, plastic, or wood
- **Underglow**: Skip the WS2812B LEDs to save on cost
- **Breadboard**: Use a smaller breadboard or solder directly to a perfboard

## Troubleshooting

See the [Help page](http://192.168.4.1/help.html) on the web interface for detailed troubleshooting information.

Common issues:

1. **Car doesn't power on**: Check battery connections and voltage
2. **Cannot connect to Wi-Fi**: Verify ESP32 is running and broadcasting
3. **Motors don't respond**: Check connections and battery level
4. **Erratic movement**: Perform motor calibration
5. **Battery drains quickly**: Check for shorts or stalled motors

## Project Structure

```
firmware/
├── main/
│   ├── main.cpp           # Main application entry point
│   ├── config.h           # System configuration and pin definitions
│   ├── motor.cpp          # Motor control implementation
│   ├── motor.h            # Motor control interface
│   ├── lights.cpp         # Lighting system implementation
│   ├── lights.h           # Lighting system interface
│   ├── battery.cpp        # Battery monitoring implementation
│   ├── battery.h          # Battery monitoring interface
│   ├── failsafe.cpp       # Failsafe mechanisms implementation
│   ├── failsafe.h         # Failsafe mechanisms interface
│   ├── webserver.cpp      # Web server and WebSocket implementation
│   ├── webserver.h        # Web server and WebSocket interface
│   └── spiffs_data/       # Web interface files
│       ├── index.html     # Main control interface
│       ├── calibration.html # Calibration page
│       ├── diagnostics.html # Diagnostics page
│       ├── update.html    # OTA update page
│       ├── help.html      # Help and documentation page
│       ├── css/
│       │   └── style.css  # Stylesheet for web interface
│       └── js/
│           ├── app.js     # Main application JavaScript
│           └── nipplejs.min.js # Joystick library
├── CMakeLists.txt         # CMake build configuration
└── partitions.csv         # Partition table for flash memory
```

## License

This project is released under the MIT License. See the LICENSE file for details.

## Acknowledgements

- [ESP-IDF](https://github.com/espressif/esp-idf) by Espressif Systems
- [nippleJS](https://github.com/yoannmoinet/nipplejs) for the virtual joystick
- All open-source libraries and tools used in this project

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Contact

For questions or support, please open an issue on the project repository.