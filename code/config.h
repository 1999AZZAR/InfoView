/*
 * Configuration and constants for ESP32-C3 Smartwatch Display
 */

#ifndef CONFIG_H
#define CONFIG_H

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Pin definitions for ESP32-C3 Super Mini
#define SDA_PIN 9
#define SCL_PIN 8
// LED_PIN removed - not used for lower power consumption

// Display mode switching intervals
#define MODE_TIME_DURATION 20000 // 20 seconds for time mode
#define MODE_WEATHER_DURATION 10000 // 10 seconds for weather mode
#define MODE_FORECAST_DURATION 10000 // 10 seconds for forecast mode
#define MODE_EYE_DURATION 15000 // 15 seconds for eye animation mode

// Notification configuration
#define MAX_NOTIFICATIONS 4
#define NOTIFICATION_DISPLAY_TIME 6000 // 6 seconds
#define NOTIFICATION_DISPLAY_TIME_NAV 3000 // 3 seconds during navigation

// Scrolling configuration
#define SCROLL_INTERVAL 300 // Scroll every 300ms
#define SCROLL_PAUSE 1000 // Pause 1 second at start/end

// Weather cache configuration
#define CACHE_MAX_AGE 3600000 // 1 hour in milliseconds

// Watchdog timer configuration
#define WDT_TIMEOUT_MS 30000 // 30 seconds

// Device name
#define DEVICE_NAME "InfoView"

// Display face enable/disable configuration
// Set to 1 to enable, 0 to disable
#define ENABLE_TIME_FACE 1
#define ENABLE_WEATHER_FACE 1
#define ENABLE_FORECAST_FACE 0
#define ENABLE_NOTIFICATION_FACE 1
#define ENABLE_NAVIGATION_FACE 1
#define ENABLE_EYE_FACE 1

#endif // CONFIG_H

