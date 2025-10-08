# ESP32 Temperature Monitor

Real-time temperature monitoring system using ESP32 and DS18B20 sensor with a modern web interface.

## Features

- **Real-time monitoring** - Updates every 100ms via WebSocket
- **Modern web interface** - Responsive design with temperature history chart
- **REST API** - `/temperature` endpoint for external integrations
- **Error handling** - Sensor disconnection and WiFi failure detection
- **Temperature history** - Visual chart with min/max tracking

## Hardware Requirements

- ESP32 development board
- DS18B20 temperature sensor
- 4.7kΩ resistor (pull-up)
- Breadboard and jumper wires

## Wiring Diagram

```
DS18B20 Sensor:
- VDD (Red)    → ESP32 3.3V
- GND (Black)  → ESP32 GND
- DATA (Yellow)→ ESP32 GPIO4 (with 4.7kΩ pull-up to 3.3V)
```

>[!Note]
> Ensure resistor is connected between DATA and VDD pins.
> You must not connect VDD to 5V as it may damage the ESP32.
> You must not connect resistor in series with DATA line.


## Software Setup

### 1. Install PlatformIO

Install [PlatformIO IDE](https://platformio.org/install/ide) or PlatformIO Core:

```bash
# Using pip
pip install platformio

# Or using homebrew (macOS)
brew install platformio
```

### 2. Clone and Setup Project

```bash
git clone "https://github.com/codebysnorlax/iot-projects.git"
cd iot-projects/ESP32/EspTemp

### 3. Configure WiFi

Edit `src/main.cpp` and update WiFi credentials:

```cpp
const char *ssid = "Your_WiFi_Name";
const char *password = "Your_WiFi_Password";
```

### 4. Upload Web Interface

The HTML file must be uploaded to ESP32's SPIFFS filesystem:

```bash
# Upload filesystem (contains index.html)
platformio run --target uploadfs

# Build and upload firmware
platformio run --target upload

# Monitor serial output
platformio device monitor
```

### 5. Access Web Interface

1. Open Serial Monitor to see ESP32's IP address
2. Open browser and navigate to: `http://ESP32_IP_ADDRESS`
3. View real-time temperature updates

## Project Structure

```
EspTemp/
├── src/
│   └── main.cpp           # Main ESP32 code
├── include/
│   └── index.html         # Web interface
├── platformio.ini         # PlatformIO configuration
└── README.md             # This file
```

## API Endpoints

### WebSocket
- **URL**: `ws://ESP32_IP/ws`
- **Data**: `{"temperature": 25.3, "status": "ok"}`

### REST API
- **GET** `/temperature`
- **Response**: `{"temperature": 25.3, "status": "ok"}`
- **Error**: `{"temperature": "Error", "status": "error"}`

## Configuration Options

### Temperature Update Rate
```cpp
#define TEMP_UPDATE_INTERVAL 100  // milliseconds
```

### Sensor Pin
```cpp
#define ONE_WIRE_BUS 4  // GPIO pin
```

### Sensor Resolution
```cpp
sensors.setResolution(9);  // 9-12 bits (9=fastest, 12=most accurate)
```

## Troubleshooting

### Build Errors
```bash
# Clean build files
rm -rf .pio
platformio run
```

### Sensor Not Detected
- Check wiring connections
- Verify 4.7kΩ pull-up resistor
- Test sensor with multimeter

### WiFi Connection Issues
- Verify SSID and password
- Check signal strength
- Try different WiFi channel

### Web Interface Not Loading
- Ensure `uploadfs` was run successfully
- Check SPIFFS mount in serial monitor
- Verify HTML file is in `include/` folder

## Serial Monitor Output

Expected output:
```
=== ESP32 Temperature Monitor ===
Found 1 DS18B20 sensor(s)
SPIFFS mounted successfully
WiFi connected!
IP Address: 192.168.1.100
Web server started
```

## Dependencies

All dependencies are automatically managed by PlatformIO:

- `OneWire` - DS18B20 communication
- `DallasTemperature` - Temperature sensor library
- `ESPAsyncWebServer` - Async web server
- `AsyncTCP` - TCP library for ESP32

## License

MIT License - Feel free to modify and distribute.
