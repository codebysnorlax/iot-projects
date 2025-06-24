# Getting Started with ESP32 Smart Car

## Uploading Firmware to ESP32

This guide covers different methods to upload the firmware to your ESP32 device.

### Prerequisites

Before you begin, make sure you have:

1. **ESP-IDF installed** - The Espressif IoT Development Framework is required to build and flash ESP32 projects
2. **USB Cable** - A micro-USB cable to connect the ESP32 to your computer
3. **ESP32 Development Board** - The ESP32 WROOM-32 C development board
4. **Git** (optional) - For cloning the repository

### Method 1: Using ESP-IDF Command Line

This is the standard method using the official ESP-IDF tools.

#### Step 1: Set up the ESP-IDF environment

**For Windows:**
```batch
cd %userprofile%\esp\esp-idf
export.bat
```

**For Linux/macOS:**
```bash
. $HOME/esp/esp-idf/export.sh
```

#### Step 2: Navigate to the project directory

```bash
cd path/to/firmware
```

#### Step 3: Build the project

```bash
idf.py build
```

#### Step 4: Connect your ESP32 to your computer

Connect the ESP32 board to your computer using a micro-USB cable.

#### Step 5: Flash the firmware

```bash
idf.py -p PORT flash
```

Replace `PORT` with your ESP32's serial port:
- Windows: `COM3` (or another COM port number)
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- macOS: `/dev/cu.SLAB_USBtoUART` or `/dev/cu.usbserial-XXX`

If you're unsure about the port, you can run:
```bash
idf.py -p auto flash
```

#### Step 6: Monitor the output (optional)

```bash
idf.py -p PORT monitor
```

### Method 2: Using Visual Studio Code with ESP-IDF Extension

This method provides a graphical interface for building and flashing.

#### Step 1: Install Visual Studio Code and the ESP-IDF Extension

1. Download and install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code and go to Extensions (Ctrl+Shift+X)
3. Search for "ESP-IDF" and install the official Espressif extension

#### Step 2: Configure the ESP-IDF Extension

1. Open the Command Palette (Ctrl+Shift+P)
2. Type "ESP-IDF: Configure ESP-IDF extension" and select it
3. Follow the wizard to set up the ESP-IDF path and tools

#### Step 3: Open the project folder

1. In VS Code, go to File > Open Folder
2. Navigate to and select the firmware folder

#### Step 4: Build and flash

1. Click on the ESP-IDF Explorer icon in the sidebar
2. Under "PROJECT TASKS", click on "Build" to compile the project
3. Connect your ESP32 to your computer
4. Select the correct serial port from the status bar at the bottom
5. Click on "Flash" to upload the firmware

### Method 3: Using ESP Flash Download Tool (Windows Only)

This is a GUI tool provided by Espressif for Windows users.

#### Step 1: Download the Flash Download Tool

Download the [Flash Download Tool](https://www.espressif.com/en/support/download/other-tools) from Espressif's website.

#### Step 2: Prepare binary files

Build the project using ESP-IDF:
```bash
idf.py build
```

#### Step 3: Launch the Flash Download Tool

1. Extract and run the downloaded tool
2. Select "ESP32" as the chip type
3. Select "Developer Mode"

#### Step 4: Configure flash settings

1. Set the correct COM port for your ESP32
2. Configure the binary files with their respective addresses:
   - `build/bootloader/bootloader.bin` at address `0x1000`
   - `build/partition_table/partition-table.bin` at address `0x8000`
   - `build/firmware.bin` at address `0x10000`

#### Step 5: Flash the ESP32

1. Click "START" to begin flashing
2. Wait for the process to complete

### Method 4: Using Arduino IDE

Arduino IDE provides a familiar and beginner-friendly environment for programming ESP32 devices.

#### Step 1: Install Arduino IDE

1. Download and install [Arduino IDE](https://www.arduino.cc/en/software) (version 1.8.x or 2.x)
2. Open Arduino IDE

#### Step 2: Add ESP32 Board Support

1. Go to **File > Preferences**
2. In the "Additional Boards Manager URLs" field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Click "OK"
4. Go to **Tools > Board > Boards Manager**
5. Search for "esp32"
6. Install the "ESP32 by Espressif Systems" package
7. Close the Boards Manager

#### Step 3: Configure Arduino IDE for ESP32

1. Go to **Tools > Board > ESP32 Arduino** and select "ESP32 Dev Module"
2. Set the following options under the Tools menu:
   - Upload Speed: "921600"
   - CPU Frequency: "240MHz (WiFi/BT)"
   - Flash Frequency: "80MHz"
   - Flash Mode: "QIO"
   - Flash Size: "4MB (32Mb)"
   - Partition Scheme: "Default 4MB with spiffs"
   - Core Debug Level: "None"
   - PSRAM: "Disabled"

#### Step 4: Prepare the Arduino Sketch

For the ESP32 Smart Car project, you'll need to adapt the ESP-IDF code to work with Arduino:

1. Create a new sketch: **File > New**
2. Save it with a meaningful name: **File > Save As**
3. Add the necessary libraries:
   ```cpp
   #include <WiFi.h>
   #include <WebServer.h>
   #include <WebSocketsServer.h>
   #include <SPIFFS.h>
   #include <ESP32PWM.h>
   ```

#### Step 5: Install Required Libraries

1. Go to **Tools > Manage Libraries**
2. Search for and install:
   - "WebSockets" by Markus Sattler
   - "ESP32PWM" by Chris O.
   - Any other libraries your project requires

#### Step 6: Upload the Code

1. Connect your ESP32 to your computer via USB
2. Select the correct port under **Tools > Port**
3. Click the "Upload" button (right arrow icon)

#### Step 7: Upload SPIFFS Data (Web Interface)

To upload the web interface files to SPIFFS:

1. Install the ESP32 Sketch Data Upload tool:
   - For Arduino IDE 1.x: Download [ESP32FS plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin/releases) and place in Arduino/tools folder
   - For Arduino IDE 2.x: Use the Arduino IDE Library Manager to install "ESP32 Sketch Data Upload"
2. Create a "data" folder in your sketch folder
3. Copy all web interface files (HTML, CSS, JS) to this folder
4. Select **Tools > ESP32 Sketch Data Upload**

### Method 5: Using Platform IO (Alternative)

Platform IO provides an alternative development environment that can be easier for beginners.

#### Step 1: Install Platform IO

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code and go to Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE" and install it

#### Step 2: Import the project

1. In VS Code, open the PlatformIO Home (click the PlatformIO icon in the sidebar)
2. Click "Open Project"
3. Navigate to and select the firmware folder

#### Step 3: Configure platformio.ini

Create or modify the `platformio.ini` file in the project root:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
monitor_speed = 115200
```

#### Step 4: Build and upload

1. Connect your ESP32 to your computer
2. Click the "Upload" button in the PlatformIO toolbar

### Troubleshooting

#### Cannot connect to ESP32

1. Check the USB cable - try a different cable
2. Verify the COM port is correct
3. Make sure you have the correct USB drivers installed
4. Press and hold the BOOT button on the ESP32 while starting the flash process

#### Flash operation failed

1. Put the ESP32 in download mode: 
   - Press and hold the BOOT button
   - Press the RST button briefly
   - Release the BOOT button
2. Try a slower baud rate: `idf.py -p PORT -b 115200 flash`

#### Permission denied error on Linux/macOS

```bash
sudo chmod 666 /dev/ttyUSB0  # Replace with your port
```

#### Build errors

1. Make sure ESP-IDF is properly installed and set up
2. Check that all dependencies are installed
3. Try cleaning the build: `idf.py fullclean`

### Next Steps

After successfully uploading the firmware:

1. Power on the smart car
2. Connect to the Wi-Fi network "ESP32_SmartCar" (password: "smartcar123")
3. Open a web browser and navigate to http://192.168.4.1
4. Follow the calibration instructions on the web interface

### Additional Resources

- [Official ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [ESP32 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [ESP32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)