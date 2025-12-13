/*
 * ESP32-C3 Super Mini BLE Weather & Navigation Display
 * Using ChronosESP32 library to interface with Chronos app
 * Features:
 * - BLE connectivity via Chronos app
 * - OLED display for weather, time, notifications, navigation
 * - Real-time data updates from Chronos app
 * 
 * Modular architecture:
 * - config.h: Configuration and constants
 * - display_manager: Display mode management
 * - display_time: Time display functions
 * - display_weather: Weather display functions
 * - display_notification: Notification display functions
 * - display_navigation: Navigation display functions
 * - notification_queue: Notification queue management
 * - weather_cache: Weather data caching
 * - ble_handler: BLE connection and callbacks
 */

#include <ChronosESP32.h>
#include <ESP32Time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_task_wdt.h"

#include "config.h"
#include "display_manager.h"
#include "notification_queue.h"
#include "weather_cache.h"
#include "ble_handler.h"

// Global objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ChronosESP32 chronos(DEVICE_NAME);
ESP32Time rtc;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize watchdog timer
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT_MS,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);

  // Initialize OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("InfoView");
  display.println("Initializing...");
  display.display();

  // Initialize modules
  initDisplayManager();
  initNotificationQueue();
  initWeatherCache();
  initBLE();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("InfoView Ready!");
  display.println("Waiting for");
  display.println("Chronos app...");
  display.display();
}

void loop() {
  // Feed watchdog timer
  esp_task_wdt_reset();

  // Handle Chronos library routine functions
  chronos.loop();
  
  // Update weather cache when new data arrives
  if (chronos.getWeatherCount() > 0) {
    // Check if this is new data (different from cache or cache is invalid)
    // Get weather once to avoid multiple calls
    Weather currentWeather = chronos.getWeatherAt(0);
    if (!cachedWeather.valid || 
        cachedWeather.temp != currentWeather.temp ||
        cachedWeather.icon != currentWeather.icon) {
      updateWeatherCache();
    }
  }

  // Update display
  updateDisplay();

  // Reduced delay for smoother updates (50ms = 20fps max refresh rate)
  delay(50);
}
