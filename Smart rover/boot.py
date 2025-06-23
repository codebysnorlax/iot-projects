# boot.py - Smart Rover Boot Script
# This script runs on ESP8266 boot and handles initial setup

import gc
import webrepl

# Garbage collection to free up memory
gc.collect()

# Disable automatic garbage collection to improve performance
# during critical operations
gc.threshold(90000)

# Enable WebREPL for remote debugging and management
webrepl.start()

print("Boot sequence complete. Starting Smart Rover...")