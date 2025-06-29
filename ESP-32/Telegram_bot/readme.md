# ESP32 Telegram Bot (OceanLabz ESP32 WROOM-32 C CP2102)

This project sends a message to a Telegram bot every 5 seconds using an ESP32 board. It is built using the Arduino framework on PlatformIO and designed for the **OceanLabz ESP32 WROOM-32 C Type CP2102 USB Dual Core** board.

---

## 📦 Hardware

* **Board**: OceanLabz ESP32 WROOM-32 (C Type CP2102)
* **USB Interface**: CP2102
* **Microcontroller**: Dual Core Xtensa LX6 (ESP32)
* **Wi-Fi Module**: Integrated 802.11 b/g/n
* **Power Supply**: USB (5V) via CP2102
* **Dev Environment**: PlatformIO + Arduino Framework
* **Driver Required (Windows)**: [CP210x Universal Windows Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

> 📥 Make sure to install the CP210x driver if your ESP32 is not recognized on Windows. The filename may look like `CP210x_Universal_Windows_Driver` as shown in the icon.

---

## 🧰 PlatformIO Configuration

Add this to your `platformio.ini` file:

```ini
[env:upesy_wroom]
platform = espressif32
board = upesy_wroom
framework = arduino
monitor_speed = 115200
```

---

## 🔐 Configuration (`config.h`)

Create a file at `include/config.h` with your WiFi and Telegram bot credentials:



## 🚀 How It Works

1. On startup, the ESP32 connects to your Wi-Fi.
2. Every 5 seconds (configurable in `interval`), it sends a message to a Telegram chat using the Bot API.
3. The device uses `WiFiClientSecure` to make HTTPS requests to `https://api.telegram.org`.
4. Response is printed to the serial monitor.

---

## 🧪 Testing Instructions

1. **Create Telegram Bot**:

   * Open Telegram, search for [@BotFather](https://t.me/BotFather)
   * Use `/newbot` command to generate a new bot and token

2. **Get Chat ID**:

   * Start a chat with your bot or search for your own chat ID
   * Use a site like `https://api.telegram.org/bot<YourToken>/getUpdates` to retrieve the chat ID

3. \*\*Update \*\***`config.h`** with token and chat ID

4. **Upload Firmware**:

   ```bash
   pio run --target upload
   pio device monitor
   ```

5. **Expected Output**:

   ```
   Connecting to WiFi.....
   Connected to WiFi
   Sending message to: https://api.telegram.org/bot<your_token>/sendMessage?chat_id=<your_chat_id>&text=Hello from ESP32
   HTTP Response code: 200
   Response payload: {"ok":true, ...}
   ```

---

## 🛠️ Customization

You can modify the message or add dynamic values:

```cpp
String message = "Temperature: " + String(temperature) + " C";
```

Use sensors like DHT11/DHT22 or BMP180 to send real-time environmental data.

---

## 🔒 Security Tips

* Avoid using `client.setInsecure()` in production — use `setCACert()` for proper SSL certs.
* Protect your `BOT_TOKEN` as it grants control over the bot.
* Consider using an `.env` parser or secret manager for larger projects.

---

## ❓ Troubleshooting

* **ESP32 not showing up on Windows**:

  * Ensure CP210x drivers are installed
  * Try different USB cables or ports
  * Use Device Manager to check COM port

* **Stuck on Connecting to WiFi**:

  * Double check SSID and password in `config.h`
  * Ensure 2.4GHz WiFi is enabled

* **No response from Telegram API**:

  * Check that the bot is started and you're sending to a valid chat ID
  * Monitor HTTPS errors in Serial output

* **Code upload fails**:

  * Board not in flash mode or busy
  * Try holding `BOOT` button when uploading

---

## 🧠 Credits

Made with ❤️ by [Ravi Ranjan Sharma](https://www.instagram.com/nr_snorlax/") using OceanLabz ESP32, PlatformIO, and the Telegram Bot API.

---
