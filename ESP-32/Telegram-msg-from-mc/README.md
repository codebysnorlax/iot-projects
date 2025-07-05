# ESP-32 Telegram Bot

A simple IoT project that connects an ESP-32 microcontroller to Telegram to send periodic messages.

## Project Overview

This project demonstrates how to use an ESP-32 microcontroller to connect to WiFi and send periodic messages to a Telegram chat using the Telegram Bot API. The ESP-32 connects to a WiFi network, and every 5 seconds, it sends a "Hello from ESP-32" message to a specified Telegram chat.

## Hardware Requirements

- ESP-32 development board (UPesy WROOM model, as specified in platformio.ini)
- USB cable for programming and power
- Computer with PlatformIO installed

## Software Dependencies

- PlatformIO
- Arduino framework for ESP-32
- WiFi library (built-in)
- HTTPClient library (built-in)
- WiFiClientSecure library (built-in)
- ArduinoJson library (v7.4.2) - Used for parsing JSON responses from Telegram API

## Project Structure

```
├── .gitignore           # Git ignore file (excludes config.h for security)
├── .vscode/
│   └── extensions.json   # Recommended VS Code extensions
├── include/
│   ├── README           # Information about header files
│   └── config.h         # Configuration file with WiFi and Telegram credentials
├── lib/
│   └── README           # Information about project libraries
├── platformio.ini       # PlatformIO configuration
├── src/
│   └── main.cpp         # Main application code
└── test/
    └── README           # Information about unit testing
```

## Configuration

Before running the project, you need to configure your WiFi and Telegram credentials in the `include/config.h` file:

```cpp
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASS "YourWiFiPassword"
#define BOT_TOKEN "YourTelegramBotToken"
#define CHAT_ID "YourTelegramChatID"
```

### How to Get Telegram Bot Token and Chat ID

1. **Create a Telegram Bot**:
   - Open Telegram and search for "BotFather"
   - Start a chat with BotFather and send the command `/newbot`
   - Follow the instructions to create a new bot
   - Once created, BotFather will provide you with a token

2. **Get Your Chat ID**:
   - Start a chat with your new bot
   - Send a message to your bot
   - Visit `https://api.telegram.org/bot<YourBOTToken>/getUpdates`
   - Look for the "id" field in the "chat" object

## Building and Uploading

1. Open the project in PlatformIO
2. Connect your ESP-32 to your computer
3. Build and upload the project using PlatformIO
4. Open the serial monitor to view debug information (baud rate: 115200, configured in platformio.ini)

## How It Works

1. The ESP-32 connects to the configured WiFi network
2. Every 5 seconds, it sends a message to the specified Telegram chat
3. If the WiFi connection is lost, it attempts to reconnect
4. The serial monitor displays connection status and API response information

## Code Explanation

- **Setup Function**: Initializes serial communication and connects to WiFi
- **Loop Function**: Periodically sends messages to Telegram
- **WiFi Connection**: Uses the WiFi library to establish and maintain a connection
- **HTTP Requests**: Uses HTTPClient to send GET requests to the Telegram API
- **Secure Connection**: Uses WiFiClientSecure with insecure mode to skip certificate validation

## Security Considerations

- The current implementation uses `client.setInsecure()` which skips SSL certificate validation. For production use, consider implementing proper certificate validation.
- Credentials are stored in a header file. For better security, consider using more secure storage methods.
- The `config.h` file is included in `.gitignore` to prevent accidentally committing sensitive credentials to version control.

## Future Improvements

- Add the ability to receive and process messages from Telegram
- Implement sensor readings and send them to Telegram
- Add commands to control ESP-32 functions remotely
- Improve security by implementing proper certificate validation

## License

This project is open-source and available for personal and educational use.

## Author

[Snorlax | Ravi Ranjan Sharma](https://www.instagram.com/nr_snorlax/)

---

*Note: Remember to update the config.h file with your own WiFi and Telegram credentials before uploading the code to your ESP-32.*