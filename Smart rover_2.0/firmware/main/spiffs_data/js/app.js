/**
 * ESP32 Smart Car Control Interface
 * Main JavaScript file for the web interface
 */

// WebSocket connection
let socket;
let reconnectInterval;
let isConnected = false;
let joystick;
let lastSentControl = { left: 0, right: 0 };
let controlUpdateInterval;

// UI state
let activeTab = 'control';
let headlightsOn = false;
let taillightsOn = false;
let indicatorState = 0; // 0: off, 1: left, 2: right, 3: hazard
let hornOn = false;
let underglowColor = { r: 0, g: 255, b: 0 };
let underglowMode = 0;

// Constants
const INDICATOR_OFF = 0;
const INDICATOR_LEFT = 1;
const INDICATOR_RIGHT = 2;
const INDICATOR_HAZARD = 3;

// DOM Elements
document.addEventListener('DOMContentLoaded', () => {
    // Initialize the interface
    initWebSocket();
    initJoystick();
    initEventListeners();
    
    // Prevent scrolling on touch devices
    document.body.addEventListener('touchmove', (e) => {
        if (e.target.closest('.settings-panel') === null) {
            e.preventDefault();
        }
    }, { passive: false });
    
    // Set up control update interval
    controlUpdateInterval = setInterval(sendControlUpdate, 100);
});

/**
 * Initialize WebSocket connection
 */
function initWebSocket() {
    // Close any existing socket
    if (socket) {
        socket.close();
    }
    
    // Clear any existing reconnect interval
    if (reconnectInterval) {
        clearInterval(reconnectInterval);
    }
    
    // Get the WebSocket URL
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;
    
    // Create a new WebSocket connection
    socket = new WebSocket(wsUrl);
    
    // WebSocket event handlers
    socket.onopen = () => {
        console.log('WebSocket connected');
        isConnected = true;
        updateConnectionStatus(true);
        showToast('Connected to ESP32', 'success');
    };
    
    socket.onclose = () => {
        console.log('WebSocket disconnected');
        isConnected = false;
        updateConnectionStatus(false);
        
        // Attempt to reconnect
        reconnectInterval = setInterval(() => {
            if (!isConnected) {
                initWebSocket();
            }
        }, 5000);
    };
    
    socket.onerror = (error) => {
        console.error('WebSocket error:', error);
        isConnected = false;
        updateConnectionStatus(false);
    };
    
    socket.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            updateUI(data);
        } catch (error) {
            console.error('Error parsing WebSocket message:', error);
        }
    };
}

/**
 * Initialize the joystick control
 */
function initJoystick() {
    const joystickZone = document.getElementById('joystick-zone');
    
    joystick = nipplejs.create({
        zone: joystickZone,
        mode: 'static',
        position: { left: '50%', top: '50%' },
        color: 'var(--primary-color)',
        size: 120
    });
    
    joystick.on('move', (event, data) => {
        // Calculate motor speeds based on joystick position
        // Forward/backward is controlled by Y axis
        // Left/right turning is controlled by X axis
        
        const forward = -data.vector.y; // Invert Y axis so up is forward
        const turn = data.vector.x;
        
        // Tank-style control: differential steering
        let leftSpeed = Math.round(100 * (forward - turn));
        let rightSpeed = Math.round(100 * (forward + turn));
        
        // Clamp values to [-100, 100]
        leftSpeed = Math.max(-100, Math.min(100, leftSpeed));
        rightSpeed = Math.max(-100, Math.min(100, rightSpeed));
        
        // Update the UI
        updateSpeedIndicators(leftSpeed, rightSpeed);
        
        // Store the values to be sent in the next update
        lastSentControl.left = leftSpeed;
        lastSentControl.right = rightSpeed;
    });
    
    joystick.on('end', () => {
        // Stop the motors when joystick is released
        lastSentControl.left = 0;
        lastSentControl.right = 0;
        updateSpeedIndicators(0, 0);
    });
}

/**
 * Initialize event listeners for UI elements
 */
function initEventListeners() {
    // Tab buttons
    document.getElementById('control-tab-btn').addEventListener('click', () => switchTab('control'));
    document.getElementById('lights-tab-btn').addEventListener('click', () => switchTab('lights'));
    document.getElementById('settings-tab-btn').addEventListener('click', () => switchTab('settings'));
    
    // Action buttons
    document.getElementById('headlights-btn').addEventListener('click', toggleHeadlights);
    document.getElementById('taillights-btn').addEventListener('click', toggleTaillights);
    document.getElementById('indicators-btn').addEventListener('click', toggleIndicatorControls);
    document.getElementById('horn-btn').addEventListener('touchstart', () => setHorn(true));
    document.getElementById('horn-btn').addEventListener('touchend', () => setHorn(false));
    document.getElementById('horn-btn').addEventListener('mousedown', () => setHorn(true));
    document.getElementById('horn-btn').addEventListener('mouseup', () => setHorn(false));
    document.getElementById('horn-btn').addEventListener('mouseleave', () => setHorn(false));
    
    // Indicator buttons
    document.getElementById('left-indicator-btn').addEventListener('click', () => setIndicator(INDICATOR_LEFT));
    document.getElementById('right-indicator-btn').addEventListener('click', () => setIndicator(INDICATOR_RIGHT));
    document.getElementById('hazard-indicator-btn').addEventListener('click', () => setIndicator(INDICATOR_HAZARD));
    document.getElementById('off-indicator-btn').addEventListener('click', () => setIndicator(INDICATOR_OFF));
    
    // Underglow controls
    document.getElementById('underglow-color').addEventListener('input', updateUnderglowColor);
    document.getElementById('underglow-mode').addEventListener('change', updateUnderglowMode);
    
    // Settings buttons
    document.getElementById('calibrate-motors-btn').addEventListener('click', calibrateMotors);
    document.getElementById('calibrate-battery-btn').addEventListener('click', showBatteryCalibrationModal);
    document.getElementById('reset-failsafe-btn').addEventListener('click', resetFailsafe);
    document.getElementById('check-update-btn').addEventListener('click', checkForUpdates);
    
    // Battery calibration modal
    document.getElementById('cancel-battery-cal-btn').addEventListener('click', hideBatteryCalibrationModal);
    document.getElementById('confirm-battery-cal-btn').addEventListener('click', calibrateBattery);
}

/**
 * Switch between tabs (control, lights, settings)
 */
function switchTab(tab) {
    activeTab = tab;
    
    // Update tab button states
    document.querySelectorAll('.menu-button').forEach(btn => btn.classList.remove('active'));
    document.getElementById(`${tab}-tab-btn`).classList.add('active');
    
    // Show/hide settings panel
    const settingsPanel = document.getElementById('settings-panel');
    if (tab === 'settings') {
        settingsPanel.classList.remove('hidden');
        setTimeout(() => settingsPanel.classList.add('visible'), 10);
    } else {
        settingsPanel.classList.remove('visible');
        setTimeout(() => settingsPanel.classList.add('hidden'), 300);
    }
    
    // Show/hide indicator controls
    const indicatorControls = document.getElementById('indicator-controls');
    if (tab === 'lights') {
        indicatorControls.classList.remove('hidden');
        document.getElementById('underglow-controls').classList.remove('hidden');
    } else {
        indicatorControls.classList.add('hidden');
        document.getElementById('underglow-controls').classList.add('hidden');
    }
}

/**
 * Toggle headlights on/off
 */
function toggleHeadlights() {
    headlightsOn = !headlightsOn;
    document.getElementById('headlights-btn').classList.toggle('active', headlightsOn);
    sendLightsUpdate();
}

/**
 * Toggle taillights on/off
 */
function toggleTaillights() {
    taillightsOn = !taillightsOn;
    document.getElementById('taillights-btn').classList.toggle('active', taillightsOn);
    sendLightsUpdate();
}

/**
 * Toggle indicator controls visibility
 */
function toggleIndicatorControls() {
    switchTab('lights');
}

/**
 * Set indicator state
 */
function setIndicator(state) {
    indicatorState = state;
    
    // Update button states
    document.querySelectorAll('.indicator-button').forEach(btn => btn.classList.remove('active'));
    
    switch (state) {
        case INDICATOR_LEFT:
            document.getElementById('left-indicator-btn').classList.add('active');
            break;
        case INDICATOR_RIGHT:
            document.getElementById('right-indicator-btn').classList.add('active');
            break;
        case INDICATOR_HAZARD:
            document.getElementById('hazard-indicator-btn').classList.add('active');
            break;
        case INDICATOR_OFF:
            document.getElementById('off-indicator-btn').classList.add('active');
            break;
    }
    
    sendLightsUpdate();
}

/**
 * Set horn state
 */
function setHorn(state) {
    hornOn = state;
    document.getElementById('horn-btn').classList.toggle('active', hornOn);
    sendHornUpdate();
}

/**
 * Update underglow color from color picker
 */
function updateUnderglowColor(event) {
    const hexColor = event.target.value;
    
    // Convert hex to RGB
    const r = parseInt(hexColor.substr(1, 2), 16);
    const g = parseInt(hexColor.substr(3, 2), 16);
    const b = parseInt(hexColor.substr(5, 2), 16);
    
    underglowColor = { r, g, b };
    sendUnderglowUpdate();
}

/**
 * Update underglow mode
 */
function updateUnderglowMode(event) {
    underglowMode = parseInt(event.target.value);
    sendUnderglowUpdate();
}

/**
 * Calibrate motors
 */
function calibrateMotors() {
    fetch('/api/calibrate/motor')
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Motors calibrated successfully', 'success');
            } else {
                showToast('Motor calibration failed', 'error');
            }
        })
        .catch(error => {
            console.error('Error calibrating motors:', error);
            showToast('Error calibrating motors', 'error');
        });
}

/**
 * Show battery calibration modal
 */
function showBatteryCalibrationModal() {
    document.getElementById('battery-calibration-modal').classList.remove('hidden');
}

/**
 * Hide battery calibration modal
 */
function hideBatteryCalibrationModal() {
    document.getElementById('battery-calibration-modal').classList.add('hidden');
}

/**
 * Calibrate battery with measured voltage
 */
function calibrateBattery() {
    const measuredVoltage = document.getElementById('measured-voltage').value;
    
    fetch(`/api/calibrate/battery?voltage=${measuredVoltage}`)
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Battery calibrated successfully', 'success');
            } else {
                showToast('Battery calibration failed', 'error');
            }
            hideBatteryCalibrationModal();
        })
        .catch(error => {
            console.error('Error calibrating battery:', error);
            showToast('Error calibrating battery', 'error');
            hideBatteryCalibrationModal();
        });
}

/**
 * Reset failsafe
 */
function resetFailsafe() {
    fetch('/api/reset')
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Failsafe reset successfully', 'success');
            } else {
                showToast('Failsafe reset failed', 'error');
            }
        })
        .catch(error => {
            console.error('Error resetting failsafe:', error);
            showToast('Error resetting failsafe', 'error');
        });
}

/**
 * Check for updates
 */
function checkForUpdates() {
    // This would be implemented if OTA updates are enabled
    showToast('No updates available', 'info');
}

/**
 * Send motor control update to the server
 */
function sendControlUpdate() {
    if (!isConnected) return;
    
    const message = {
        type: 'control',
        left: lastSentControl.left,
        right: lastSentControl.right
    };
    
    socket.send(JSON.stringify(message));
}

/**
 * Send lights update to the server
 */
function sendLightsUpdate() {
    if (!isConnected) return;
    
    const message = {
        type: 'lights',
        headlights: headlightsOn,
        taillights: taillightsOn,
        indicators: indicatorState
    };
    
    socket.send(JSON.stringify(message));
}

/**
 * Send horn update to the server
 */
function sendHornUpdate() {
    if (!isConnected) return;
    
    const message = {
        type: 'horn',
        state: hornOn
    };
    
    socket.send(JSON.stringify(message));
}

/**
 * Send underglow update to the server
 */
function sendUnderglowUpdate() {
    if (!isConnected) return;
    
    const message = {
        type: 'underglow',
        r: underglowColor.r,
        g: underglowColor.g,
        b: underglowColor.b,
        mode: underglowMode
    };
    
    socket.send(JSON.stringify(message));
}

/**
 * Update the UI with data received from the server
 */
function updateUI(data) {
    // Update battery status
    if (data.battery) {
        const batteryLevel = document.getElementById('battery-level');
        const batteryPercentage = document.getElementById('battery-percentage');
        const batteryVoltage = document.getElementById('battery-voltage');
        
        batteryLevel.style.width = `${data.battery.percentage}%`;
        batteryPercentage.textContent = `${data.battery.percentage}%`;
        batteryVoltage.textContent = `${data.battery.voltage.toFixed(1)}V`;
        
        // Update battery level color
        batteryLevel.classList.remove('warning', 'critical');
        if (data.battery.critical) {
            batteryLevel.classList.add('critical');
        } else if (data.battery.low) {
            batteryLevel.classList.add('warning');
        }
    }
    
    // Update system status
    if (data.failsafe) {
        const systemStatus = document.getElementById('system-status');
        
        if (data.failsafe.active) {
            systemStatus.textContent = 'FAILSAFE';
            systemStatus.classList.add('danger');
            
            // Show failsafe reason
            let reason = 'Unknown';
            switch (data.failsafe.reason) {
                case 1: reason = 'Communication Timeout'; break;
                case 2: reason = 'Critical Battery'; break;
                case 3: reason = 'Motor Blocked'; break;
                case 4: reason = 'Overcurrent'; break;
            }
            
            showToast(`Failsafe: ${reason}`, 'error');
        } else {
            systemStatus.textContent = 'Normal';
            systemStatus.classList.remove('danger');
        }
    }
    
    // Update uptime
    if (data.uptime !== undefined) {
        const hours = Math.floor(data.uptime / 3600);
        const minutes = Math.floor((data.uptime % 3600) / 60);
        const seconds = Math.floor(data.uptime % 60);
        
        document.getElementById('system-uptime').textContent = 
            `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
    }
    
    // Update version
    if (data.version) {
        document.getElementById('system-version').textContent = data.version;
    }
    
    // Update motor speeds
    if (data.motors) {
        updateSpeedIndicators(data.motors.leftSpeed, data.motors.rightSpeed);
    }
    
    // Update light states
    if (data.lights) {
        // Only update if not actively controlling
        if (data.lights.headlights !== undefined && headlightsOn !== data.lights.headlights) {
            headlightsOn = data.lights.headlights;
            document.getElementById('headlights-btn').classList.toggle('active', headlightsOn);
        }
        
        if (data.lights.taillights !== undefined && taillightsOn !== data.lights.taillights) {
            taillightsOn = data.lights.taillights;
            document.getElementById('taillights-btn').classList.toggle('active', taillightsOn);
        }
        
        if (data.lights.indicators !== undefined && indicatorState !== data.lights.indicators) {
            setIndicator(data.lights.indicators);
        }
        
        // Update underglow color
        if (data.lights.underglow) {
            const { r, g, b } = data.lights.underglow;
            const hexColor = `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
            document.getElementById('underglow-color').value = hexColor;
        }
    }
    
    // Update horn state
    if (data.horn !== undefined) {
        hornOn = data.horn;
        document.getElementById('horn-btn').classList.toggle('active', hornOn);
    }
    
    // Update connection status
    if (data.connected !== undefined) {
        updateConnectionStatus(data.connected);
    }
}

/**
 * Update the speed indicators
 */
function updateSpeedIndicators(leftSpeed, rightSpeed) {
    // Update left speed indicator
    const leftBar = document.getElementById('left-speed-bar');
    const leftValue = document.getElementById('left-speed-value');
    
    const absLeftSpeed = Math.abs(leftSpeed);
    leftBar.style.height = `${absLeftSpeed}%`;
    leftValue.textContent = `${leftSpeed}%`;
    leftBar.classList.toggle('reverse', leftSpeed < 0);
    
    // Update right speed indicator
    const rightBar = document.getElementById('right-speed-bar');
    const rightValue = document.getElementById('right-speed-value');
    
    const absRightSpeed = Math.abs(rightSpeed);
    rightBar.style.height = `${absRightSpeed}%`;
    rightValue.textContent = `${rightSpeed}%`;
    rightBar.classList.toggle('reverse', rightSpeed < 0);
}

/**
 * Update the connection status indicator
 */
function updateConnectionStatus(connected) {
    const connectionIcon = document.getElementById('connection-icon');
    const connectionText = document.getElementById('connection-text');
    
    if (connected) {
        connectionIcon.classList.remove('disconnected');
        connectionIcon.classList.add('connected');
        connectionText.textContent = 'Connected';
    } else {
        connectionIcon.classList.remove('connected');
        connectionIcon.classList.add('disconnected');
        connectionText.textContent = 'Disconnected';
    }
}

/**
 * Show a toast message
 */
function showToast(message, type = 'info') {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = 'toast';
    toast.classList.add(type);
    toast.classList.remove('hidden');
    
    // Hide the toast after 3 seconds
    setTimeout(() => {
        toast.classList.add('hidden');
    }, 3000);
}