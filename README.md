# InfoView - ESP32-C3 Smart Display

A smartwatch-style display system using ESP32-C3 Super Mini microcontroller with OLED display. The device connects to the Chronos mobile application via Bluetooth Low Energy (BLE) to display time, weather information, notifications, and navigation instructions.

## Project Structure

```
infoview/
├── code/
│   ├── code.ino              # Main firmware (setup and loop)
│   ├── config.h              # Configuration constants
│   ├── display_manager.cpp/h # Display mode management
│   ├── display_time.cpp/h    # Time display functions
│   ├── display_weather.cpp/h# Weather display functions
│   ├── display_notification.cpp/h # Notification display
│   ├── display_navigation.cpp/h   # Navigation display
│   ├── notification_queue.cpp/h   # Notification queue
│   ├── weather_cache.cpp/h        # Weather data cache
│   └── ble_handler.cpp/h          # BLE connection handlers
├── platformio.ini        # PlatformIO configuration (optional)
├── LICENSE                # MIT License
├── .gitignore            # Git ignore rules
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

1. **Time Mode (MODE_TIME)**: Watch face displaying current time and date
2. **Weather Mode (MODE_WEATHER)**: Weather information including temperature, high/low, pressure, UV index, and location
3. **Notification Mode (MODE_NOTIFICATION)**: Incoming notifications from connected mobile device
4. **Navigation Mode (MODE_NAVIGATION)**: Turn-by-turn navigation instructions with directional arrows

### Mode Switching Logic

- Time and Weather modes alternate automatically every 12 seconds
- Weather mode is automatically skipped if no weather data is available (current or cached)
- Notifications interrupt the normal cycle and display for 6 seconds (3 seconds during navigation)
- Navigation mode immediately overrides the time/weather loop when active and displays continuously
- Navigation updates every 500ms for smooth real-time display
- After notifications or navigation, the system returns to the time/weather cycle
- Smooth transitions with brief dim effect when switching between display modes

### Timing Configuration

- Time/Weather switching interval: 12 seconds
- Notification display duration: 6 seconds (normal), 3 seconds (during navigation)
- Navigation update rate: 500ms for smooth real-time updates
- Display update rate: Optimized (updates only when content changes or every 1 second for time mode)
- Main loop delay: 50ms for smooth 20fps refresh rate
- Smooth transitions with brief dim effect on mode changes

### BLE Communication

The firmware uses the ChronosESP32 library which implements a standardized BLE protocol for communication with the Chronos mobile application. The device advertises as "InfoView" and implements the following features:

- Time synchronization from mobile device
- Weather data reception
- Notification forwarding
- Navigation data reception
- Connection status monitoring

### Display Functions

#### Time Display
- Time display: hh:mm:ss format (size 2 font, centered on one line)
- Date display: Full day name + DD/MM/YYYY format (centered)
- Top decorative line: Phone battery indicator (proportional line length, full line = 100% battery)
- Bottom decorative line: Connection status indicator (only visible when connected to Chronos app)
- Clean, minimal design with functional status indicators

#### Weather Display
- Layout: 40/60 split design
- Left 40%: Weather icon (36x36 pixels) and weather description text (e.g., "Clear", "Sunny", "Cloudy", "Rain", "Snow", "Storm", "Fog", "Drizzle")
- Right 60%: Current temperature (size 2, centered), UV index and Pressure (centered), High/Low temperatures (centered)
- Top header: City name with scrolling for long names (inverted colors)
- Bottom bar: Date and time in DD/MM hh:mm format (inverted colors, centered)
- Weather description: Text description based on ChronosESP32 weather icon code
- Weather icons: Detailed 36x36 pixel pixel art icons for different weather conditions
- Time-based icon calculation: Icons automatically switch between day and night variants based on current time (6 AM - 6 PM = day, 6 PM - 6 AM = night)
  - Clear weather (icon 0): Sun during day, moon and stars at night
  - Other weather conditions: Same icon for day and night
- Icon code system: Uses ChronosESP32 icon codes (0-9) only
  - 0: Clear (day: sun, night: moon and stars)
  - 1: Sunny/Partly Cloudy
  - 2: Cloudy
  - 3: Rain (light rain)
  - 4: Rain (heavy rain)
  - 5: Storm
  - 6: Snow
  - 7: Fog
  - 8: Drizzle
  - 9: Cloudy/Overcast
- UV index: Displayed as "UV:X" format
- Pressure: Displayed as "P:XXX" format (truncated to 3 characters)
- Offline support: Uses cached weather data when connection is lost (valid for 1 hour)

#### Notification Display
- Top line: Application name (left) and notification count (right, e.g., "1/4")
- Top separator line below header
- Content: Up to 5 lines of message content with word wrapping (uses full screen height for maximum content space)
- Text truncation: Adds "..." ellipsis on last line if content doesn't fit
- Notification queue: Stores up to 4 notifications
- Clean layout: No icons, no bottom separator, focused on content readability

#### Navigation Display
- Layout: 40/60 split design
- Left 40%: Large directional arrow (40x40, centered vertically)
- Right 60%: Instruction text, distance to next turn, total distance, and duration
- Text wrapping: All text uses full height with word wrapping for long content
- Vertical divider line between arrow and text areas

## Usage

### Initial Setup

1. Power on the ESP32-C3 device
2. The device will initialize and display "InfoView Ready! Waiting for Chronos app..."
3. The device advertises as "InfoView" via BLE

### Connecting with Chronos App

1. Install Chronos app on your Android device
2. Open Chronos app
3. Navigate to the watch section
4. Tap "Pair New Watch" or the "+" button
5. Select "InfoView" from the available devices list
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

- Displays time for 12 seconds
- Automatically switches to weather for 12 seconds (only if weather data is available)
- If no weather data is available, stays on time mode
- Continuously cycles between time and weather when data is available
- Weather data is cached for offline operation (valid for 1 hour)
- Smooth transitions between display modes with optimized refresh rate

### Notification Reception

- When a notification arrives, it immediately interrupts the current display
- Notification displays for 6 seconds (normal operation)
- During navigation, notifications display for 3 seconds to minimize interruption
- After display time expires, returns to time/weather cycle or navigation

### Navigation Active

- When navigation is active, navigation display immediately overrides the time/weather loop
- Navigation displays continuously while active with 500ms update rate
- Notifications during navigation display for shorter duration (3 seconds) to minimize interruption
- Returns to time/weather cycle immediately when navigation ends

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
- Look for "InfoView" in the available devices list

**Connection fails or disconnects frequently**
- Check for Bluetooth interference
- Ensure devices are within range (typically 10 meters)
- Verify Chronos app has necessary permissions

**No data appears on display**
- Verify connection is established (check display for connection status)
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
- Verify BLE connection is still active
- Restart the device

### Data Issues

**No weather data**
- Configure weather location in Chronos app
- Verify location services are enabled
- Check Chronos app has internet connection
- Wait a few minutes for initial data sync

**Weather icon or description incorrect**
- The system uses ChronosESP32 icon codes (0-9) from the Chronos app
- Icon codes are automatically mapped to appropriate descriptions and icons
- Clear weather icons automatically switch between day (sun) and night (moon/stars) based on current time
- Day/night calculation: 6 AM - 6 PM = day, 6 PM - 6 AM = night
- If weather shows incorrectly, verify the Chronos app is displaying the correct weather condition

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

- Microcontroller: ESP32-C3 (RISC-V, 80MHz, 400KB SRAM)
- Display: SSD1306 OLED, 128x64 pixels, I2C interface (100kHz)
- Communication: Bluetooth Low Energy 5.0
- Power: USB 5V (via USB-C or micro-USB depending on board variant)
- Power optimizations:
  - CPU frequency: 80MHz (reduced from 160MHz for lower power and heat)
  - Flash frequency: 40MHz (reduced from 80MHz)
  - I2C speed: 100kHz (optimized for OLED)
  - LED indicator removed (not needed, saves power)

### Software Specifications

- Framework: Arduino (ESP32 Arduino Core 3.3.4)
- BLE Stack: NimBLE
- Display Driver: Adafruit SSD1306
- Graphics Library: Adafruit GFX
- Protocol: ChronosESP32 BLE protocol

### Memory Usage

- Program storage: ~680KB (52% of 1.3MB available) - Optimized by removing redundant code
- Dynamic memory: ~30KB (9% of 327KB available) - Optimized by eliminating unnecessary String objects
- Local cache: Weather data cached for offline operation (minimal memory overhead)
- Code optimizations:
  - Removed all Serial debugging code (no Serial port initialization overhead)
  - Eliminated redundant library calls (single weather data fetch instead of multiple)
  - Removed unnecessary String object creation (direct integer calculations)
  - Standardized to ChronosESP32 0-9 icon format only (removed ~185 lines of fallback code)
- Power optimizations:
  - CPU frequency reduced to 80MHz (50% power reduction)
  - Flash frequency reduced to 40MHz (50% power reduction)
  - LED hardware removed (saves ~5-10mA)
  - I2C clock speed optimized to 100kHz
  - Estimated total power savings: ~40-50% compared to default settings

## Development

### Code Structure

- Modular architecture with separate files for each component:
  - `code.ino`: Main setup and loop functions
  - `config.h`: Configuration constants and pin definitions
  - `display_manager.cpp/h`: Display mode management and switching logic
  - `display_time.cpp/h`: Time display functions
  - `display_weather.cpp/h`: Weather display functions with icons, ChronosESP32 icon code mapping (0-9), and time-based day/night icon calculation
  - `display_notification.cpp/h`: Notification display functions
  - `display_navigation.cpp/h`: Navigation display with arrow drawing
  - `notification_queue.cpp/h`: Notification queue management
  - `weather_cache.cpp/h`: Weather data caching for offline operation
  - `ble_handler.cpp/h`: BLE connection and callback handlers
- Main loop handles display updates and mode switching
- Callback functions handle BLE events (connection, notifications)
- Optimized display updates with smooth transitions
- Performance optimizations:
  - Single weather data fetch per update cycle (no redundant calls)
  - Direct integer calculations for display positioning (no String concatenation)
  - Minimal memory allocations (reused variables, no temporary String objects)
  - Efficient icon handling (0-9 format only, no fallback code paths)
  - Cached library calls (navigation state, connection state)
- Power optimizations:
  - CPU frequency: 80MHz (configured in platformio.ini and code.ino)
  - Flash frequency: 40MHz (configured in platformio.ini)
  - I2C speed: 100kHz (explicitly set in code.ino)
  - LED hardware: Removed (not needed for operation)

### Customization

To modify display timing:
- Edit `MODE_SWITCH_INTERVAL` constant in `config.h` for time/weather switching
- Edit `NOTIFICATION_DISPLAY_TIME` constant in `config.h` for notification duration
- Edit `NOTIFICATION_DISPLAY_TIME_NAV` constant in `config.h` for notification duration during navigation

To change device name:
- Edit `DEVICE_NAME` in `config.h`

To modify pin assignments:
- Edit pin definitions in `config.h` (SDA_PIN, SCL_PIN)

To adjust display smoothness:
- Edit main loop delay in `code.ino` (currently 50ms)
- Modify transition effects in `display_manager.cpp`


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
