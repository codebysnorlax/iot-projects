// For my little fish :(

/*
 * ESP8266 Autonomous Fish Tank Pump Controller
 * 
 * Features:
 * - Runs pump for 2 minutes every 30 minutes during day cycle
 * - Runs pump for 1 minute every 60 minutes during night cycle
 * - 16-hour day cycle, 8-hour night cycle
 * - No WiFi, no external sensors
 * - Deep sleep for power efficiency
 * - Automatic cycle reset every 24 hours
 * 
 * Hardware:
 * - ESP8266 GPIO2 connected to MOSFET gate (through 1kΩ resistor)
 * - MOSFET controls 3-4V pump
 * - 10kΩ pull-down resistor on MOSFET gate
 */

#include <ESP8266WiFi.h>

// Pin definitions
#define PUMP_PIN 2  // GPIO2 (D4 on NodeMCU)
#define LED_PIN 1   // GPIO1 (TX pin) for status indication

// Timing constants (in milliseconds)
#define DAY_PUMP_INTERVAL    (30UL * 60 * 1000)  // 30 minutes
#define NIGHT_PUMP_INTERVAL  (60UL * 60 * 1000)  // 60 minutes
#define DAY_PUMP_DURATION    (2UL * 60 * 1000)   // 2 minutes
#define NIGHT_PUMP_DURATION  (1UL * 60 * 1000)   // 1 minute
#define DAY_CYCLE_DURATION   (16UL * 60 * 60 * 1000)  // 16 hours
#define NIGHT_CYCLE_DURATION (8UL * 60 * 60 * 1000)   // 8 hours
#define TOTAL_CYCLE_DURATION (24UL * 60 * 60 * 1000)  // 24 hours

// Deep sleep durations (in microseconds)
#define SLEEP_30_MIN (30UL * 60 * 1000000)  // 30 minutes
#define SLEEP_60_MIN (60UL * 60 * 1000000)  // 60 minutes
#define SLEEP_1_MIN  (1UL * 60 * 1000000)   // 1 minute for pump duration

// RTC memory structure to persist data across deep sleep
struct {
  uint32_t crc32;
  uint32_t cycleStartTime;
  uint32_t lastPumpTime;
  uint16_t bootCount;
  bool isFirstBoot;
} rtcData;

// Function prototypes
void runPump(unsigned long duration);
void enterDeepSleep(unsigned long sleepTime);
uint32_t calculateCRC32(const uint8_t *data, size_t length);
bool readRTCMemory();
void writeRTCMemory();
void blinkStatus(int times);
unsigned long getCurrentCycleTime();
bool isDayTime();

void setup() {
  // Disable WiFi completely
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  
  // Initialize pins
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);  // Pump off initially
  digitalWrite(LED_PIN, LOW);   // LED off initially
  
  // Brief startup indication
  blinkStatus(3);
  
  // Try to read persistent data from RTC memory
  if (!readRTCMemory()) {
    // First boot or invalid data - initialize
    rtcData.cycleStartTime = millis();
    rtcData.lastPumpTime = 0;
    rtcData.bootCount = 1;
    rtcData.isFirstBoot = true;
    writeRTCMemory();
  } else {
    rtcData.bootCount++;
    rtcData.isFirstBoot = false;
    writeRTCMemory();
  }
  
  // Get current time in cycle
  unsigned long currentCycleTime = getCurrentCycleTime();
  bool dayTime = isDayTime();
  
  // Determine if it's time to run the pump
  unsigned long timeSinceLastPump = currentCycleTime - rtcData.lastPumpTime;
  unsigned long pumpInterval = dayTime ? DAY_PUMP_INTERVAL : NIGHT_PUMP_INTERVAL;
  unsigned long pumpDuration = dayTime ? DAY_PUMP_DURATION : NIGHT_PUMP_DURATION;
  
  // Check if it's time to run the pump
  if (rtcData.isFirstBoot || timeSinceLastPump >= pumpInterval) {
    // Time to run the pump
    blinkStatus(dayTime ? 2 : 1);  // 2 blinks for day, 1 for night
    
    runPump(pumpDuration);
    
    // Update last pump time
    rtcData.lastPumpTime = getCurrentCycleTime();
    writeRTCMemory();
    
    // Calculate sleep time until next pump cycle
    unsigned long sleepTime = dayTime ? SLEEP_30_MIN : SLEEP_60_MIN;
    enterDeepSleep(sleepTime);
  } else {
    // Not time for pump yet, sleep until next check
    unsigned long timeUntilNextPump = pumpInterval - timeSinceLastPump;
    unsigned long sleepTime = min(timeUntilNextPump * 1000, // Convert to microseconds
                                 dayTime ? SLEEP_30_MIN : SLEEP_60_MIN);
    enterDeepSleep(sleepTime);
  }
}

void loop() {
  // This should never be reached due to deep sleep
  delay(1000);
}

void runPump(unsigned long duration) {
  // Turn on pump
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);  // LED on during pump operation
  
  // Run for specified duration with periodic status checks
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    delay(5000);  // Check every 5 seconds
    // Toggle LED to show activity
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  
  // Turn off pump
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  delay(1000);  // Brief delay after pump operation
}

void enterDeepSleep(unsigned long sleepTime) {
  // Ensure pins are in correct state before sleep
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Enter deep sleep
  ESP.deepSleep(sleepTime, WAKE_RF_DISABLED);
}

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

bool readRTCMemory() {
  if (ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData))) {
    uint32_t crcOfData = calculateCRC32(((uint8_t*)&rtcData) + 4, sizeof(rtcData) - 4);
    if (crcOfData == rtcData.crc32) {
      return true;
    }
  }
  return false;
}

void writeRTCMemory() {
  rtcData.crc32 = calculateCRC32(((uint8_t*)&rtcData) + 4, sizeof(rtcData) - 4);
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
}

void blinkStatus(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

unsigned long getCurrentCycleTime() {
  // Calculate elapsed time since cycle start
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - rtcData.cycleStartTime;
  
  // Handle 24-hour cycle reset
  if (elapsedTime >= TOTAL_CYCLE_DURATION) {
    rtcData.cycleStartTime = currentTime;
    rtcData.lastPumpTime = 0;  // Reset pump timing
    writeRTCMemory();
    elapsedTime = 0;
  }
  
  return elapsedTime;
}

bool isDayTime() {
  unsigned long cycleTime = getCurrentCycleTime();
  return (cycleTime < DAY_CYCLE_DURATION);
}