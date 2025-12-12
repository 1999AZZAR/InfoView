# ESP32-C3 Smartwatch Display

A smartwatch-style display system using ESP32-C3 Super Mini microcontroller with OLED display. The device connects to the Chronos mobile application via Bluetooth Low Energy (BLE) to display time, weather information, notifications, and navigation instructions.

## Project Structure

```
mochi/
├── code/
│   └── code.ino          # Main firmware source code
├── platformio.ini        # PlatformIO configuration (optional)
└── README.md             # This file
```

## Hardware Requirements

- ESP32-C3 Super Mini development board
- 0.96" OLED display module (SSD1306, I2C interface)
- Connecting wires (4 wires: VCC, GND, SDA, SCL)
- USB cable for programming and power

### Pin Connections

| OLED Display | ESP32-C3 Super Mini |
|--------------|---------------------|
| VCC          | 3.3V                |
| GND          | GND                 |
| SDA          | GPIO 9              |
| SCL          | GPIO 8              |

Additional: LED indicator on GPIO 10 (optional, for connection status)

## Software Requirements

### Required Libraries

The firmware requires the following Arduino libraries:

- **ChronosESP32** (by fbiego) - BLE communication with Chronos app
- **ESP32Time** (by fbiego) - Time management (dependency of ChronosESP32)
- **NimBLE-Arduino** (by h2zero) - BLE stack (dependency of ChronosESP32)
- **Adafruit GFX Library** - Graphics library for display
- **Adafruit SSD1306** - OLED display driver

### Mobile Application

- **Chronos App** - Available on Google Play Store
  - Download: [Chronos on Google Play](https://play.google.com/store/apps/details?id=com.chronos.ke)
  - Website: [chronos.ke](https://chronos.ke)

## Installation

### Method 1: Using Arduino CLI

1. Install Arduino CLI if not already installed
2. Install ESP32 board support:
   ```bash
   arduino-cli core install esp32:esp32
   ```
3. Install required libraries:
   ```bash
   arduino-cli lib install "ChronosESP32" "ESP32Time" "Adafruit GFX Library" "Adafruit SSD1306"
   ```
4. Compile the firmware:
   ```bash
   cd code
   arduino-cli compile --fqbn esp32:esp32:esp32c3:PartitionScheme=default code.ino
   ```
5. Upload to board:
   ```bash
   arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32c3:PartitionScheme=default code.ino
   ```
   Replace `/dev/ttyACM0` with your actual serial port.

### Method 2: Using Arduino IDE

1. Install Arduino IDE (version 1.8.19 or later, or Arduino IDE 2.x)
2. Add ESP32 board support:
   - Go to File > Preferences
   - Add to Additional Board Manager URLs: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Go to Tools > Board > Boards Manager
   - Search for "esp32" and install "esp32 by Espressif Systems"
3. Install libraries:
   - Go to Sketch > Include Library > Manage Libraries
   - Install: ChronosESP32, Adafruit GFX Library, Adafruit SSD1306
4. Select board:
   - Tools > Board > ESP32 Arduino > ESP32C3 Dev Module
   - Tools > Partition Scheme > Default 4MB with spiffs (or Default)
5. Open `code/code.ino` in Arduino IDE
6. Connect ESP32-C3 via USB
7. Select the correct port: Tools > Port
8. Upload: Sketch > Upload

### Method 3: Using PlatformIO

1. Install PlatformIO IDE or PlatformIO Core
2. Open the project folder in PlatformIO
3. The `platformio.ini` file is already configured
4. Build and upload using PlatformIO interface or CLI:
   ```bash
   pio run -t upload
   ```

## Code Architecture

### Display Modes

The firmware implements four distinct display modes:

1. **Time Mode (MODE_TIME)**: Watch face displaying current time, date, and connection status
2. **Weather Mode (MODE_WEATHER)**: Weather information including temperature, high/low, pressure, UV index, and location
3. **Notification Mode (MODE_NOTIFICATION)**: Incoming notifications from connected mobile device
4. **Navigation Mode (MODE_NAVIGATION)**: Turn-by-turn navigation instructions with directional arrows

### Mode Switching Logic

- Time and Weather modes alternate automatically every 15 seconds
- Weather mode is automatically skipped if no weather data is available (current or cached)
- Notifications interrupt the normal cycle and display for 6 seconds
- Navigation mode takes priority when active and displays continuously
- After notifications or navigation, the system returns to the time/weather cycle

### Timing Configuration

- Time/Weather switching interval: 15 seconds
- Notification display duration: 6 seconds
- Display update rate: Optimized (updates only when content changes or every 1 second for time mode)

### BLE Communication

The firmware uses the ChronosESP32 library which implements a standardized BLE protocol for communication with the Chronos mobile application. The device advertises as "Mochi-Display" and implements the following features:

- Time synchronization from mobile device
- Weather data reception
- Notification forwarding
- Navigation data reception
- Connection status monitoring

### Display Functions

#### Time Display
- Time display: hh:mm:ss format (size 2 font, centered on one line)
- Date display: 5-character day name + DD/MM/YYYY format (centered)
- Connection status: "Connected" or "Waiting..." text
- Phone battery: Battery percentage and charging indicator (when available)
- Decorative top and bottom borders

#### Weather Display
- Layout: 40/60 split design
- Left 40%: Weather icon (24x24 pixels) and weather description text (e.g., "Clear", "Sunny", "Cloudy", "Rain", "Snow", "Storm", "Fog")
- Right 60%: Current temperature (size 2, centered), High/Low temperatures (centered)
- Top header: City name with scrolling for long names (inverted colors)
- Bottom bar: Date and time in DD/MM hh:mm format (inverted colors, centered)
- Weather description: Text description based on weather icon code
- Weather icons: Detailed 24x24 pixel icons for different weather conditions
- Offline support: Uses cached weather data when connection is lost (valid for 1 hour)

#### Notification Display
- Notification number: Shows current position in queue (e.g., "1/4")
- Application name: Centered below number
- Content: Up to 3 lines of message content with word wrapping
- Text truncation: Adds "..." ellipsis on last line if content doesn't fit
- Notification queue: Stores up to 4 notifications
- Clean layout: No icons, focused on content readability

#### Navigation Display
- Layout: 40/60 split design
- Left 40%: Large directional arrow (40x40, centered vertically)
- Right 60%: Instruction text, distance to next turn, total distance, and duration
- Text wrapping: All text uses full height with word wrapping for long content
- Vertical divider line between arrow and text areas

## Usage

### Initial Setup

1. Power on the ESP32-C3 device
2. The device will initialize and display "Mochi Ready! Waiting for Chronos app..."
3. The device advertises as "Mochi-Display" via BLE

### Connecting with Chronos App

1. Install Chronos app on your Android device
2. Open Chronos app
3. Navigate to the watch section
4. Tap "Pair New Watch" or the "+" button
5. Select "Mochi-Display" from the available devices list
6. Grant required permissions when prompted:
   - Bluetooth permissions
   - Location permissions (for weather data)
   - Notification access (for notifications)

### Enabling Notification Access

1. Go to Android Settings > Accessibility > Notification access
2. Enable "Chronos"
3. Return to Chronos app

### Configuring Weather

1. Open Chronos app settings
2. Configure weather location
3. Enable weather updates
4. Weather data will appear on the display

### Using Navigation

1. Start navigation in any maps application (Google Maps, OsmAnd, etc.)
2. Ensure Chronos app has notification access enabled
3. Navigation instructions will automatically appear on the display
4. The display shows turn-by-turn directions with arrows

## Display Behavior

### Normal Operation

- Displays time for 15 seconds
- Automatically switches to weather for 15 seconds (only if weather data is available)
- If no weather data is available, stays on time mode
- Continuously cycles between time and weather when data is available
- Weather data is cached for offline operation (valid for 1 hour)

### Notification Reception

- When a notification arrives, it immediately interrupts the current display
- Notification displays for 6 seconds
- After 6 seconds, returns to time/weather cycle

### Navigation Active

- When navigation is active, navigation display takes priority
- Navigation displays continuously while active
- Returns to time/weather cycle when navigation ends

## Troubleshooting

### Compilation Errors

**Error: Partition table not found**
- Solution: Ensure board is selected as "ESP32C3 Dev Module" (not regular ESP32)
- Set Partition Scheme to "Default" in Arduino IDE

**Error: Library not found**
- Solution: Install all required libraries via Library Manager
- Verify library names match exactly: ChronosESP32, ESP32Time, Adafruit GFX Library, Adafruit SSD1306

**Error: BLE-related compilation errors**
- Solution: Ensure ESP32 board package version 3.3.4 or later is installed
- Update board package if using older version

### Connection Issues

**Device not found in Chronos app**
- Verify ESP32-C3 is powered and code is uploaded
- Check Bluetooth is enabled on mobile device
- Restart both devices
- Check serial monitor for BLE advertising messages

**Connection fails or disconnects frequently**
- Check for Bluetooth interference
- Ensure devices are within range (typically 10 meters)
- Verify Chronos app has necessary permissions
- Check serial monitor for error messages

**No data appears on display**
- Verify connection is established (check LED indicator if connected)
- Ensure Chronos app is configured correctly
- Check notification access is enabled
- Verify location services are enabled for weather

### Display Issues

**Blank or garbled display**
- Verify OLED connections (SDA=GPIO 9, SCL=GPIO 8)
- Check I2C address is 0x3C (default for most SSD1306 modules)
- Verify power supply is stable (3.3V)
- Test with I2C scanner sketch to confirm display detection

**Text not displaying correctly**
- Verify screen dimensions are set correctly (128x64)
- Check font size settings in code
- Ensure display library version is compatible

**Display freezes or stops updating**
- Check serial monitor for error messages
- Verify BLE connection is still active
- Restart the device

### Data Issues

**No weather data**
- Configure weather location in Chronos app
- Verify location services are enabled
- Check Chronos app has internet connection
- Wait a few minutes for initial data sync

**No notifications**
- Enable notification access in Android Settings
- Verify Chronos app has notification access permission
- Check that notifications are enabled in source applications
- Test with a simple notification (SMS, etc.)

**No navigation data**
- Ensure navigation is active in maps application
- Verify notification access is enabled
- Check that maps app is sending navigation notifications
- Some navigation apps may not be compatible

## Technical Specifications

### Hardware Specifications

- Microcontroller: ESP32-C3 (RISC-V, 160MHz, 400KB SRAM)
- Display: SSD1306 OLED, 128x64 pixels, I2C interface
- Communication: Bluetooth Low Energy 5.0
- Power: USB 5V (via USB-C or micro-USB depending on board variant)

### Software Specifications

- Framework: Arduino (ESP32 Arduino Core 3.3.4)
- BLE Stack: NimBLE
- Display Driver: Adafruit SSD1306
- Graphics Library: Adafruit GFX
- Protocol: ChronosESP32 BLE protocol

### Memory Usage

- Program storage: ~730KB (55% of 1.3MB available)
- Dynamic memory: ~35KB (10% of 327KB available)
- Local cache: Weather data cached for offline operation (minimal memory overhead)

## Development

### Code Structure

- Main loop handles display updates and mode switching
- Callback functions handle BLE events (connection, notifications)
- Separate display functions for each mode
- Helper functions for arrow drawing and text formatting

### Customization

To modify display timing:
- Edit `MODE_SWITCH_INTERVAL` constant (line 42) for time/weather switching
- Edit `NOTIFICATION_DISPLAY_TIME` constant (line 48) for notification duration

To change device name:
- Edit the device name in `ChronosESP32 chronos("Mochi-Display");` (line 29)

To modify pin assignments:
- Edit pin definitions at top of file (lines 23-25)

### Serial Debug Output

The firmware outputs debug information to serial port at 115200 baud:
- Connection status
- Notification reception
- Navigation updates
- Error messages

Monitor serial output for troubleshooting:
```bash
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

## License

This project is open source and available under the MIT License.

Copyright (c) 2025 Azzar Budiyanto

See [LICENSE](LICENSE) file for full license text.

## References

- ChronosESP32 Library: https://github.com/fbiego/chronos-esp32
- Chronos App: https://chronos.ke
- ESP32-C3 Documentation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/
- Adafruit SSD1306 Library: https://github.com/adafruit/Adafruit_SSD1306

## Contributing

Contributions are welcome. Please ensure code follows the existing style and includes appropriate comments.
