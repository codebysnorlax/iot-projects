import network
import time

SSID = "your_ssid"   // your router ssid
PASSWORD = "your_password"  // your router password

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    
    if not wlan.isconnected():
        print("Connecting to Wi-Fi...")
        wlan.connect(SSID, PASSWORD)

        timeout = 10  # Wait 10 seconds for connection
        while not wlan.isconnected() and timeout > 0:
            time.sleep(1)
            timeout -= 1
            print("Retrying...")

    if wlan.isconnected():
        print("Connected! IP Address:", wlan.ifconfig()[0])
    else:
        print("Failed to connect. Check SSID/Password.")

connect_wifi()
