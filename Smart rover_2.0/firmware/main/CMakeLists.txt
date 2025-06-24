idf_component_register(
    SRCS 
        "main.cpp"
        "motor.cpp"
        "lights.cpp"
        "battery.cpp"
        "failsafe.cpp"
        "webserver.cpp"
    INCLUDE_DIRS ".")

# Create a SPIFFS image from the contents of the 'spiffs_data' directory
# that can be flashed to the ESP32 along with the app binary.
spiffs_create_partition_image(spiffs spiffs_data FLASH_IN_PROJECT)