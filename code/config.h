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

// Notification configuration
#define MAX_NOTIFICATIONS 4
#define NOTIFICATION_DISPLAY_TIME 6000 // 6 seconds
#define NOTIFICATION_DISPLAY_TIME_NAV 3000 // 3 seconds during navigation

// Scrolling configuration
#define SCROLL_INTERVAL 300 // Scroll every 300ms
#define SCROLL_PAUSE 2000 // Pause 2 seconds at start/end

// Weather cache configuration
#define CACHE_MAX_AGE 3600000 // 1 hour in milliseconds

// Watchdog timer configuration
#define WDT_TIMEOUT_MS 30000 // 30 seconds

// Device name
#define DEVICE_NAME "InfoView"

#endif // CONFIG_H

