# Technical Knowledge Base for ESP32 Smart Car

This document covers the relevant math, physics, and engineering principles behind the ESP32 Smart Car project. It's designed to help you understand the theoretical aspects of the project and make informed decisions during the build and operation phases.

## Table of Contents
1. [DC Motor Fundamentals](#dc-motor-fundamentals)
2. [Motor Control with PWM](#motor-control-with-pwm)
3. [Battery Technology and Management](#battery-technology-and-management)
4. [Vehicle Dynamics](#vehicle-dynamics)
5. [Electronics Fundamentals](#electronics-fundamentals)
6. [ESP32 Dual-Core Optimization](#esp32-dual-core-optimization)
7. [Wireless Communication](#wireless-communication)
8. [Safety Engineering](#safety-engineering)

## DC Motor Fundamentals

### Torque vs. RPM Relationship

DC motors exhibit an inverse relationship between torque and speed (RPM):

- **Maximum Torque**: Occurs at stall (0 RPM)
- **Maximum Speed**: Occurs at no load (minimum torque)

The relationship can be approximated by:

\[ T = T_{stall} \times (1 - \frac{\omega}{\omega_{no-load}}) \]

Where:
- \(T\) is the torque at a given speed
- \(T_{stall}\) is the stall torque
- \(\omega\) is the current angular velocity
- \(\omega_{no-load}\) is the no-load angular velocity

### Motor Current Draw

The current draw of a DC motor is proportional to the torque it produces:

\[ I = I_{no-load} + \frac{T}{K_T} \]

Where:
- \(I\) is the current
- \(I_{no-load}\) is the no-load current
- \(T\) is the torque
- \(K_T\) is the torque constant (Nm/A)

At stall conditions, the motor draws maximum current:

\[ I_{stall} = I_{no-load} + \frac{T_{stall}}{K_T} \]

This is important for sizing your motor driver and power supply.

### Motor Power and Efficiency

The mechanical power output of a motor is:

\[ P_{mech} = T \times \omega \]

Where:
- \(P_{mech}\) is the mechanical power in watts
- \(T\) is the torque in Nm
- \(\omega\) is the angular velocity in rad/s

The electrical power input is:

\[ P_{elec} = V \times I \]

The efficiency is:

\[ \eta = \frac{P_{mech}}{P_{elec}} \times 100\% \]

DC motors are most efficient at around 70-80% of their no-load speed.

## Motor Control with PWM

### PWM Principles

Pulse Width Modulation (PWM) controls the average power delivered to a motor by rapidly switching the power on and off:

\[ V_{avg} = D \times V_{supply} \]

Where:
- \(V_{avg}\) is the average voltage seen by the motor
- \(D\) is the duty cycle (0 to 1)
- \(V_{supply}\) is the supply voltage

### PWM Frequency Considerations

The optimal PWM frequency depends on the motor's electrical time constant:

\[ \tau_e = \frac{L}{R} \]

Where:
- \(\tau_e\) is the electrical time constant
- \(L\) is the motor inductance
- \(R\) is the motor resistance

For most small DC motors, a PWM frequency between 5-20 kHz works well, balancing between switching losses and motor performance.

### Dead Time

When switching motor direction, it's important to insert a small delay (dead time) between turning off one direction and enabling the other to prevent shoot-through in the H-bridge:

\[ t_{dead} \approx 1-10 \mu s \]

This prevents damage to the motor driver.

## Battery Technology and Management

### Battery Capacity

Battery capacity is typically measured in ampere-hours (Ah) or milliampere-hours (mAh):

\[ Runtime (hours) = \frac{Capacity (Ah)}{Current Draw (A)} \]

For example, a 2000mAh battery powering a system drawing 500mA will last approximately:

\[ Runtime = \frac{2 Ah}{0.5 A} = 4 hours \]

In practice, this is affected by temperature, discharge rate, and battery age.

### Voltage Drop Under Load

Batteries experience voltage drop under load due to internal resistance:

\[ V_{terminal} = V_{open-circuit} - I \times R_{internal} \]

Where:
- \(V_{terminal}\) is the voltage at the battery terminals
- \(V_{open-circuit}\) is the battery voltage with no load
- \(I\) is the current being drawn
- \(R_{internal}\) is the battery's internal resistance

This is why battery voltage decreases as the motors draw more current.

### Battery State of Charge Estimation

For Li-ion batteries, the relationship between voltage and state of charge (SoC) is non-linear. A simplified approximation for a 3.7V Li-ion cell:

- 4.2V: 100% charged
- 3.7V: ~50% charged
- 3.4V: ~20% charged
- 3.0V: Fully discharged (stop using to prevent damage)

For more accurate estimation, use a lookup table or polynomial approximation based on the discharge curve of your specific battery.

### Battery Protection

Li-ion batteries require protection against:

1. **Overcharge**: Limit charging to 4.2V per cell
2. **Over-discharge**: Stop discharge at 3.0V per cell
3. **Overcurrent**: Limit current to battery's C-rating
4. **Short circuit**: Immediate current cutoff
5. **Temperature**: Operate between 0-45°C

## Vehicle Dynamics

### Wheel Traction

The maximum traction force between wheels and surface is:

\[ F_{traction} = \mu \times m \times g \]

Where:
- \(F_{traction}\) is the maximum force before slipping
- \(\mu\) is the coefficient of friction
- \(m\) is the mass on the wheel
- \(g\) is the acceleration due to gravity

Typical coefficients of friction:
- Rubber on concrete (dry): 0.6-0.85
- Rubber on wood: 0.5-0.7
- Rubber on carpet: 0.5-0.8
- Rubber on smooth tile: 0.3-0.5

### Turning Radius

For a tank-style drive system with track width \(W\) and differential speed \(\Delta v\):

\[ R = \frac{W \times (v_L + v_R)}{2 \times (v_R - v_L)} \]

Where:
- \(R\) is the turning radius
- \(W\) is the distance between the left and right wheels
- \(v_L\) is the left side speed
- \(v_R\) is the right side speed

For a point turn (spinning in place), \(v_L = -v_R\), making \(R = 0\).

### Motor Torque Requirements

The torque required to accelerate the vehicle:

\[ T_{required} = \frac{m \times a \times r_{wheel}}{n_{motors} \times \eta_{drivetrain}} \]

Where:
- \(T_{required}\) is the required torque per motor
- \(m\) is the vehicle mass
- \(a\) is the desired acceleration
- \(r_{wheel}\) is the wheel radius
- \(n_{motors}\) is the number of driven motors
- \(\eta_{drivetrain}\) is the drivetrain efficiency

## Electronics Fundamentals

### Voltage Divider for Battery Monitoring

To measure battery voltage with the ESP32's ADC (which has a maximum input of 3.3V):

\[ V_{out} = V_{battery} \times \frac{R_2}{R_1 + R_2} \]

Where:
- \(V_{out}\) is the voltage at the ADC pin
- \(V_{battery}\) is the battery voltage
- \(R_1\) and \(R_2\) are the resistor values

For example, to measure a 12V battery with a 3.3V ADC, you might use:
- \(R_1 = 10k\Omega\)
- \(R_2 = 3.3k\Omega\)

This gives \(V_{out} = 12V \times \frac{3.3k}{10k + 3.3k} \approx 3.0V\) at full battery.

### Current Limiting for LEDs

The resistor value needed for an LED:

\[ R = \frac{V_{supply} - V_{forward}}{I_{desired}} \]

Where:
- \(R\) is the resistor value
- \(V_{supply}\) is the supply voltage
- \(V_{forward}\) is the LED forward voltage (typically 1.8-3.3V depending on color)
- \(I_{desired}\) is the desired current (typically 15-20mA for standard LEDs)

### Power Dissipation

Power dissipated in a component:

\[ P = V \times I = I^2 \times R = \frac{V^2}{R} \]

This is important for selecting appropriate resistors (typically 1/4W for signal applications) and ensuring motor drivers have adequate cooling.

## ESP32 Dual-Core Optimization

### Core Distribution

The ESP32 has two cores that can be utilized effectively:

- **Core 0**: Traditionally handles Wi-Fi and Bluetooth stack
- **Core 1**: Typically used for application code

However, you can distribute tasks across both cores for better performance:

```c
// Pin a task to Core 0
xTaskCreatePinnedToCore(
    taskFunction,   // Function to implement the task
    "TaskName",    // Name of the task
    10000,          // Stack size in words
    NULL,           // Task input parameter
    1,              // Priority of the task
    &TaskHandle,    // Task handle
    0);             // Core where the task should run (0)
```

### Task Priority

FreeRTOS task priorities range from 0 (lowest) to configMAX_PRIORITIES-1 (highest):

- Critical real-time tasks: High priority (e.g., motor control)
- Background tasks: Low priority (e.g., telemetry logging)

### Memory Considerations

The ESP32 has different memory types:

- **DRAM**: 520KB (data RAM, fast access)
- **IRAM**: Part of the 520KB (instruction RAM for time-critical code)
- **RTC RAM**: 16KB (retained during deep sleep)
- **Flash**: External (typically 4MB+, slower access)

Place frequently accessed variables in DRAM and time-critical code in IRAM for best performance.

## Wireless Communication

### Wi-Fi Performance

Factors affecting Wi-Fi performance:

- **Distance**: Signal strength decreases with square of distance
- **Obstacles**: Walls and objects attenuate signal
- **Interference**: Other devices on same channel reduce throughput
- **Power consumption**: Higher transmit power increases range but reduces battery life

### WebSocket Latency

Typical latency components for WebSocket communication:

- **Network latency**: 5-50ms depending on conditions
- **Processing time**: 1-10ms on ESP32
- **Browser rendering**: 5-20ms

Total round-trip latency: 10-80ms for responsive control.

### Data Rate Optimization

To minimize latency and bandwidth:

- Use binary WebSocket messages instead of text
- Send delta updates rather than full state
- Batch non-critical updates
- Implement client-side prediction for smoother control

## Safety Engineering

### Overcurrent Protection

Methods to protect against overcurrent:

1. **Software monitoring**: Measure current with ADC and shut down if exceeded
2. **Current limiting resistors**: For LED circuits
3. **PTC fuses**: Self-resetting protection
4. **Current sense resistors**: Low-value resistors (0.1-1Ω) to measure current

### Motor Stall Detection

Detect motor stalls by monitoring:

1. **Current consumption**: Stalled motors draw high current
2. **Encoder feedback**: If available, no movement despite power
3. **Back-EMF sensing**: Reduced back-EMF during stall

Implement a stall protection algorithm:
```
if (motor_current > stall_threshold && motor_speed < min_speed && motor_power > min_power)
    motor_stalled = true;
```

### Thermal Management

Components generate heat during operation:

- **Motor drivers**: Can reach 80°C+ under load
- **Voltage regulators**: Generate heat proportional to (Vin-Vout)×I
- **Motors**: Heat up with extended use

Design considerations:
- Add heat sinks to motor drivers
- Ensure adequate airflow
- Monitor temperature of critical components
- Implement thermal shutdown protection

### Fail-Safe Design Principles

1. **Watchdog timers**: Reset system if software hangs
2. **Default safe states**: Motors stop if communication lost
3. **Redundant checks**: Verify critical values with multiple methods
4. **Graceful degradation**: Reduce functionality rather than complete failure
5. **Error logging**: Record errors for debugging

Implement a communication timeout:
```
if (millis() - last_command_time > COMMAND_TIMEOUT)
    stop_all_motors();
```

### EMI/EMC Considerations

Electromagnetic Interference (EMI) can affect sensor readings and communication:

1. **Separate power and signal grounds**
2. **Use bypass capacitors** (0.1μF) near ICs
3. **Keep sensitive signal wires away from motor wires**
4. **Use twisted pairs for differential signals**
5. **Add ferrite beads on power lines to motors**