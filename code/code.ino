/*
 * ESP32-C3 Super Mini BLE Weather & Navigation Display
 * Using ChronosESP32 library to interface with Chronos app
 * Features:
 * - BLE connectivity via Chronos app
 * - OLED display for weather, time, notifications, navigation
 * - Real-time data updates from Chronos app
 */

#include <ChronosESP32.h>
#include <ESP32Time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_task_wdt.h"

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Pin definitions for ESP32-C3 Super Mini
#define SDA_PIN 9
#define SCL_PIN 8
#define LED_PIN 10

// Global objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ChronosESP32 chronos("Mochi-Display");
ESP32Time rtc;

// Display mode
enum DisplayMode {
  MODE_TIME,
  MODE_WEATHER,
  MODE_NOTIFICATION,
  MODE_NAVIGATION
};

DisplayMode currentMode = MODE_TIME;
unsigned long lastModeSwitch = 0;
const unsigned long MODE_SWITCH_INTERVAL = 15000; // 15 seconds for time/weather switching

// Notification queue system (up to 4 notifications)
#define MAX_NOTIFICATIONS 4
Notification notificationQueue[MAX_NOTIFICATIONS];
int notificationQueueCount = 0;
int currentNotificationIndex = 0;
unsigned long notificationStartTime = 0;
const unsigned long NOTIFICATION_DISPLAY_TIME = 6000; // 6 seconds

// Display update optimization
DisplayMode previousMode = MODE_TIME;
unsigned long lastDisplayUpdate = 0;
bool displayNeedsUpdate = true;

// Display contrast (0-255, default 128)
uint8_t displayContrast = 128;

// Scrolling text for weather location
String weatherLocationText = "";
unsigned long lastScrollTime = 0;
int scrollPosition = 0;
const unsigned long SCROLL_INTERVAL = 300; // Scroll every 300ms
const int SCROLL_PAUSE = 2000; // Pause 2 seconds at start/end
unsigned long scrollPauseStart = 0;
bool scrollPaused = true;

// Scrolling text for notification content
String notificationContentText = "";
unsigned long lastNotificationScrollTime = 0;
int notificationScrollPosition = 0;
bool notificationScrollPaused = true;
unsigned long notificationScrollPauseStart = 0;

// Local memory cache for offline operation
struct CachedWeather {
  bool valid;
  int temp;
  int high;
  int low;
  int icon;
  String city;
  unsigned long timestamp; // When data was cached
};

CachedWeather cachedWeather;
const unsigned long CACHE_MAX_AGE = 3600000; // 1 hour in milliseconds

// Connection callback
void onConnection(bool connected) {
  if (connected) {
    Serial.println("Chronos app connected");
    digitalWrite(LED_PIN, HIGH);
    chronos.syncRequest(); // Request time sync
  } else {
    Serial.println("Chronos app disconnected");
    digitalWrite(LED_PIN, LOW);
  }
}

// Update weather cache when new data arrives
void updateWeatherCache() {
  if (chronos.getWeatherCount() > 0) {
    Weather weather = chronos.getWeatherAt(0);
    WeatherLocation location = chronos.getWeatherLocation();
    
    cachedWeather.valid = true;
    cachedWeather.temp = weather.temp;
    cachedWeather.high = weather.high;
    cachedWeather.low = weather.low;
    cachedWeather.icon = weather.icon;
    cachedWeather.city = location.city.length() > 0 ? location.city : chronos.getWeatherCity();
    cachedWeather.timestamp = millis();
    
    Serial.println("Weather cache updated: " + cachedWeather.city + " " + String(cachedWeather.temp) + "C");
  }
}

// Check if weather data is available (current or cached)
bool hasWeatherData() {
  // Check if current data is available
  if (chronos.getWeatherCount() > 0) {
    return true;
  }
  
  // Check if cached data is available and not too old
  if (cachedWeather.valid) {
    unsigned long age = millis() - cachedWeather.timestamp;
    if (age < CACHE_MAX_AGE) {
      return true;
    }
  }
  
  return false;
}

// Notification callback
void onNotification(Notification notification) {
  // Add to queue if not full, otherwise shift and add
  if (notificationQueueCount < MAX_NOTIFICATIONS) {
    notificationQueue[notificationQueueCount] = notification;
    notificationQueueCount++;
  } else {
    // Shift queue left and add new notification at end
    for (int i = 0; i < MAX_NOTIFICATIONS - 1; i++) {
      notificationQueue[i] = notificationQueue[i + 1];
    }
    notificationQueue[MAX_NOTIFICATIONS - 1] = notification;
  }
  
  currentNotificationIndex = notificationQueueCount - 1;
  notificationStartTime = millis();
  currentMode = MODE_NOTIFICATION;
  displayNeedsUpdate = true;
  
  // Notification display will reset automatically when shown
  
  Serial.println("Notification from " + notification.app + ": " + notification.title + " (Queue: " + String(notificationQueueCount) + ")");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize watchdog timer (30 second timeout)
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);

  // Initialize OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Display initialized - contrast is set automatically by begin()

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Mochi Display");
  display.println("Initializing...");
  display.display();

  // Initialize Chronos
  chronos.setConnectionCallback(onConnection);
  chronos.setNotificationCallback(onNotification);
  chronos.setNotifyBattery(true); // Enable phone battery notifications
  chronos.begin();
  
  // Initialize weather cache
  cachedWeather.valid = false;
  cachedWeather.temp = 0;
  cachedWeather.high = 0;
  cachedWeather.low = 0;
  cachedWeather.icon = 0;
  cachedWeather.city = "";
  cachedWeather.timestamp = 0;

  Serial.println("Chronos BLE Server started. Waiting for Chronos app...");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Mochi Ready!");
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
    if (!cachedWeather.valid || 
        cachedWeather.temp != chronos.getWeatherAt(0).temp ||
        cachedWeather.icon != chronos.getWeatherAt(0).icon) {
      updateWeatherCache();
    }
  }

  // Update display
  updateDisplay();

  // Small delay to prevent overwhelming the display
  delay(100);
}

void updateDisplay() {
  unsigned long currentTime = millis();
  bool modeChanged = false;

  // Auto-switch modes based on data availability and time
  if (notificationQueueCount > 0 && 
      (currentTime - notificationStartTime) < NOTIFICATION_DISPLAY_TIME) {
    if (currentMode != MODE_NOTIFICATION) {
      currentMode = MODE_NOTIFICATION;
      modeChanged = true;
      displayNeedsUpdate = true;
    }
  } else if (chronos.isConnected() && chronos.getNavigation().active && 
             (currentTime - lastModeSwitch) > MODE_SWITCH_INTERVAL) {
    if (currentMode != MODE_NAVIGATION) {
      currentMode = MODE_NAVIGATION;
      modeChanged = true;
      lastModeSwitch = currentTime;
      displayNeedsUpdate = true;
    }
  } else {
    // Switch between time and weather every 15 seconds
    // Only switch to weather if data is available (current or cached)
    if ((currentTime - lastModeSwitch) >= MODE_SWITCH_INTERVAL) {
      if (currentMode == MODE_TIME) {
        // Only switch to weather if data is available
        if (hasWeatherData()) {
          currentMode = MODE_WEATHER;
          modeChanged = true;
        } else {
          // No weather data, stay on time mode
          lastModeSwitch = currentTime; // Reset timer to avoid constant checking
        }
      } else if (currentMode == MODE_WEATHER) {
        currentMode = MODE_TIME;
        modeChanged = true;
      } else {
        // If coming from notification/nav, start with time
        currentMode = MODE_TIME;
        modeChanged = true;
      }
      if (modeChanged) {
        lastModeSwitch = currentTime;
        displayNeedsUpdate = true;
      }
    }
    
    // Clear notification after display time and move to next
    if (notificationQueueCount > 0 && 
        (currentTime - notificationStartTime) >= NOTIFICATION_DISPLAY_TIME) {
      // Remove displayed notification from queue
      for (int i = currentNotificationIndex; i < notificationQueueCount - 1; i++) {
        notificationQueue[i] = notificationQueue[i + 1];
      }
      notificationQueueCount--;
      
      // If more notifications, show next one
      if (notificationQueueCount > 0) {
        currentNotificationIndex = 0;
        notificationStartTime = currentTime;
        currentMode = MODE_NOTIFICATION;
        displayNeedsUpdate = true;
      } else {
        currentNotificationIndex = 0;
      }
    }
  }

  // Only update display if mode changed or content needs refresh
  // For time mode, update every second for seconds counter
  // For other modes, update when mode changes or every 500ms
  bool shouldUpdate = displayNeedsUpdate || modeChanged || 
                      (currentMode == MODE_TIME && (currentTime - lastDisplayUpdate >= 1000)) ||
                      (currentMode != MODE_TIME && currentMode != previousMode);
  
  if (shouldUpdate) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    switch (currentMode) {
      case MODE_TIME:
        displayTime();
        break;
      case MODE_WEATHER:
        displayWeather();
        break;
      case MODE_NOTIFICATION:
        displayNotification();
        break;
      case MODE_NAVIGATION:
        displayNavigation();
        break;
    }

    display.display();
    previousMode = currentMode;
    lastDisplayUpdate = currentTime;
    displayNeedsUpdate = false;
  }
}

void displayTime() {
  // Modern watch face design
  
  // Top decorative line
  display.drawLine(0, 0, 127, 0, SSD1306_WHITE);
  
  // Large time display - hh:mm:ss all in one line, centered
  int hour = chronos.getHourC();
  int minute = rtc.getMinute();
  int second = rtc.getSecond();
  
  // Build complete time string hh:mm:ss
  String timeStr = "";
  if (hour < 10) timeStr += "0";
  timeStr += String(hour);
  timeStr += ":";
  if (minute < 10) timeStr += "0";
  timeStr += String(minute);
  timeStr += ":";
  if (second < 10) timeStr += "0";
  timeStr += String(second);
  
  // Center the time (size 2 to fit hh:mm:ss on one line)
  display.setTextSize(2);
  int timeWidth = timeStr.length() * 12; // Approximate width for size 2 (8 chars = hh:mm:ss = ~96px)
  int timeX = (128 - timeWidth) / 2;
  display.setCursor(timeX, 8);
  display.print(timeStr);
  
  // Date display - centered below time
  int day = rtc.getDay();
  int month = rtc.getMonth();
  int year = rtc.getYear();
  
  // Day name - 5 characters (first 5 chars of full day name)
  const char* dayNames[] = {"Sunda", "Monda", "Tuesd", "Wedne", "Thurs", "Frida", "Satur"};
  int dayOfWeek = rtc.getDayofWeek();
  
  // Build date string
  String dateStr = String(dayNames[dayOfWeek]);
  dateStr += " ";
  if (day < 10) dateStr += "0";
  dateStr += String(day);
  dateStr += "/";
  if (month < 10) dateStr += "0";
  dateStr += String(month);
  dateStr += "/";
  dateStr += String(year);
  
  // Center the date
  display.setTextSize(1);
  int dateWidth = dateStr.length() * 6; // Approximate width for size 1
  int dateX = (128 - dateWidth) / 2;
  display.setCursor(dateX, 35);
  display.print(dateStr);
  
  // Connection status and phone battery
  display.setCursor(0, 50);
  if (chronos.isConnected()) {
    display.print("Connected");
    
    // Phone battery level (if available) - just number
    if (chronos.isSubscribed()) {
      uint8_t phoneBattery = chronos.getPhoneBattery();
      bool phoneCharging = chronos.isPhoneCharging();
      
      display.setCursor(90, 50);
      display.print(phoneBattery);
      display.print("%");
      
      if (phoneCharging) {
        display.print("+");
      }
    }
  } else {
    display.print("Waiting...");
  }
  
  // Bottom decorative line
  display.drawLine(0, 63, 127, 63, SSD1306_WHITE);
}

// Get weather description text from icon code
String getWeatherDescription(int icon) {
  if (icon == 800) {
    return "Clear";
  } else if (icon == 801) {
    return "Sunny";
  } else if (icon >= 200 && icon < 300) {
    return "Storm";
  } else if (icon >= 500 && icon < 600) {
    return "Rain";
  } else if (icon >= 600 && icon < 700) {
    return "Snow";
  } else if (icon >= 700 && icon < 800) {
    return "Fog";
  } else if (icon >= 802 && icon <= 804) {
    return "Cloudy";
  } else {
    return "Cloudy";
  }
}

// Draw weather icon based on icon code
// Improved icons for better visibility on 24x24 monochrome display
void drawWeatherIcon(int icon, int x, int y) {
  // OpenWeatherMap icon codes:
  // Clear: 800 (clear sky day), 801 (few clouds: 11-25%)
  // Clouds: 802 (scattered: 25-50%), 803 (broken: 51-84%), 804 (overcast: 85-100%)
  // Rain: 500-531
  // Thunderstorm: 200-232
  // Snow: 600-622
  // Mist/fog: 701-781
  
  // Center point for 24x24 icon: x+12, y+12
  
  if (icon == 800) {
    // Clear sky - bright sun with rays
    display.fillCircle(x + 12, y + 12, 7, SSD1306_WHITE);
    // 8 rays (cardinal and diagonal)
    display.drawLine(x + 12, y + 1, x + 12, y + 4, SSD1306_WHITE);
    display.drawLine(x + 12, y + 20, x + 12, y + 23, SSD1306_WHITE);
    display.drawLine(x + 1, y + 12, x + 4, y + 12, SSD1306_WHITE);
    display.drawLine(x + 20, y + 12, x + 23, y + 12, SSD1306_WHITE);
    display.drawLine(x + 4, y + 4, x + 6, y + 6, SSD1306_WHITE);
    display.drawLine(x + 18, y + 18, x + 20, y + 20, SSD1306_WHITE);
    display.drawLine(x + 20, y + 4, x + 18, y + 6, SSD1306_WHITE);
    display.drawLine(x + 4, y + 20, x + 6, y + 18, SSD1306_WHITE);
  } else if (icon == 801) {
    // Few clouds - sun with small cloud
    display.fillCircle(x + 8, y + 8, 5, SSD1306_WHITE);
    display.drawLine(x + 8, y + 1, x + 8, y + 3, SSD1306_WHITE);
    display.drawLine(x + 1, y + 8, x + 3, y + 8, SSD1306_WHITE);
    display.drawLine(x + 4, y + 4, x + 5, y + 5, SSD1306_WHITE);
    display.drawLine(x + 13, y + 13, x + 14, y + 14, SSD1306_WHITE);
    // Small cloud
    display.fillCircle(x + 15, y + 14, 4, SSD1306_WHITE);
    display.fillCircle(x + 19, y + 14, 4, SSD1306_WHITE);
    display.fillRect(x + 14, y + 15, 7, 3, SSD1306_WHITE);
  } else if (icon >= 200 && icon < 300) {
    // Thunderstorm - cloud with lightning
    // Cloud
    display.fillCircle(x + 7, y + 6, 4, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 6, 5, SSD1306_WHITE);
    display.fillRect(x + 6, y + 8, 10, 3, SSD1306_WHITE);
    // Lightning bolt (more detailed)
    display.fillTriangle(x + 11, y + 11, x + 13, y + 11, x + 12, y + 14, SSD1306_WHITE);
    display.fillTriangle(x + 12, y + 14, x + 8, y + 17, x + 12, y + 17, SSD1306_WHITE);
    display.fillRect(x + 11, y + 17, 2, 5, SSD1306_WHITE);
    display.fillRect(x + 10, y + 19, 1, 3, SSD1306_WHITE);
  } else if (icon >= 500 && icon < 600) {
    // Rain - cloud with rain drops
    // Cloud
    display.fillCircle(x + 6, y + 6, 4, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 6, 5, SSD1306_WHITE);
    display.fillRect(x + 5, y + 8, 11, 3, SSD1306_WHITE);
    // Rain drops (vertical lines for better visibility)
    display.drawLine(x + 7, y + 13, x + 7, y + 18, SSD1306_WHITE);
    display.drawLine(x + 10, y + 14, x + 10, y + 19, SSD1306_WHITE);
    display.drawLine(x + 14, y + 13, x + 14, y + 18, SSD1306_WHITE);
    display.drawLine(x + 17, y + 14, x + 17, y + 19, SSD1306_WHITE);
  } else if (icon >= 600 && icon < 700) {
    // Snow - cloud with detailed snowflake
    // Cloud
    display.fillCircle(x + 6, y + 6, 4, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 6, 5, SSD1306_WHITE);
    display.fillRect(x + 5, y + 8, 11, 3, SSD1306_WHITE);
    // Detailed snowflake (6-pointed)
    int sx = x + 12, sy = y + 16;
    display.drawLine(sx, sy - 4, sx, sy + 4, SSD1306_WHITE);
    display.drawLine(sx - 4, sy, sx + 4, sy, SSD1306_WHITE);
    display.drawLine(sx - 3, sy - 3, sx + 3, sy + 3, SSD1306_WHITE);
    display.drawLine(sx + 3, sy - 3, sx - 3, sy + 3, SSD1306_WHITE);
    // Small dots at ends
    display.fillCircle(sx, sy - 4, 1, SSD1306_WHITE);
    display.fillCircle(sx, sy + 4, 1, SSD1306_WHITE);
    display.fillCircle(sx - 4, sy, 1, SSD1306_WHITE);
    display.fillCircle(sx + 4, sy, 1, SSD1306_WHITE);
  } else if (icon >= 700 && icon < 800) {
    // Mist/fog - wavy horizontal lines
    display.drawLine(x + 3, y + 6, x + 21, y + 6, SSD1306_WHITE);
    display.drawLine(x + 4, y + 9, x + 20, y + 9, SSD1306_WHITE);
    display.drawLine(x + 3, y + 12, x + 21, y + 12, SSD1306_WHITE);
    display.drawLine(x + 5, y + 15, x + 19, y + 15, SSD1306_WHITE);
    display.drawLine(x + 4, y + 18, x + 20, y + 18, SSD1306_WHITE);
  } else if (icon == 802) {
    // Scattered clouds - sun partially visible with clouds
    // Partial sun
    display.fillCircle(x + 7, y + 7, 4, SSD1306_WHITE);
    display.drawLine(x + 7, y + 1, x + 7, y + 3, SSD1306_WHITE);
    display.drawLine(x + 1, y + 7, x + 3, y + 7, SSD1306_WHITE);
    // Clouds covering part of sun
    display.fillCircle(x + 11, y + 9, 4, SSD1306_WHITE);
    display.fillCircle(x + 16, y + 9, 4, SSD1306_WHITE);
    display.fillRect(x + 10, y + 10, 8, 3, SSD1306_WHITE);
  } else if (icon == 803) {
    // Broken clouds - mostly clouds with small sun
    // Small sun
    display.fillCircle(x + 5, y + 5, 3, SSD1306_WHITE);
    display.drawLine(x + 5, y + 1, x + 5, y + 2, SSD1306_WHITE);
    display.drawLine(x + 1, y + 5, x + 2, y + 5, SSD1306_WHITE);
    // Larger clouds
    display.fillCircle(x + 10, y + 8, 5, SSD1306_WHITE);
    display.fillCircle(x + 17, y + 8, 5, SSD1306_WHITE);
    display.fillRect(x + 9, y + 10, 11, 4, SSD1306_WHITE);
  } else if (icon == 804) {
    // Overcast - full cloud cover
    display.fillCircle(x + 6, y + 7, 5, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 7, 6, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 8, 4, SSD1306_WHITE);
    display.fillRect(x + 5, y + 9, 14, 5, SSD1306_WHITE);
  } else {
    // Default - generic cloud
    display.fillCircle(x + 6, y + 7, 5, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 7, 6, SSD1306_WHITE);
    display.fillRect(x + 5, y + 9, 12, 4, SSD1306_WHITE);
  }
}

// Helper function to handle scrolling text
void updateScrollingText(String text, int maxWidth) {
  unsigned long currentTime = millis();
  
  // Reset scroll if text changed
  if (text != weatherLocationText) {
    weatherLocationText = text;
    scrollPosition = 0;
    scrollPaused = true;
    scrollPauseStart = currentTime;
    return;
  }
  
  // If text fits, no need to scroll
  if (text.length() <= maxWidth) {
    scrollPosition = 0;
    scrollPaused = true;
    return;
  }
  
  // Handle pause at start and end
  if (scrollPaused) {
    if (currentTime - scrollPauseStart >= SCROLL_PAUSE) {
      scrollPaused = false;
      lastScrollTime = currentTime;
    }
    return;
  }
  
  // Check if we've scrolled to the end
  if (scrollPosition >= text.length() - maxWidth) {
    // Pause at end, then reset
    if (currentTime - scrollPauseStart >= SCROLL_PAUSE) {
      scrollPosition = 0;
      scrollPauseStart = currentTime;
    } else if (scrollPosition == text.length() - maxWidth) {
      scrollPauseStart = currentTime;
    }
    return;
  }
  
  // Scroll text
  if (currentTime - lastScrollTime >= SCROLL_INTERVAL) {
    scrollPosition++;
    lastScrollTime = currentTime;
  }
}

void displayWeather() {
  Weather weather;
  WeatherLocation location;
  String city;
  bool useCache = false;
  
  // Try to get current weather data
  if (chronos.getWeatherCount() > 0) {
    weather = chronos.getWeatherAt(0);
    location = chronos.getWeatherLocation();
    city = location.city.length() > 0 ? location.city : chronos.getWeatherCity();
  } else if (cachedWeather.valid) {
    // Use cached data if available and not too old
    unsigned long age = millis() - cachedWeather.timestamp;
    if (age < CACHE_MAX_AGE) {
      weather.temp = cachedWeather.temp;
      weather.high = cachedWeather.high;
      weather.low = cachedWeather.low;
      weather.icon = cachedWeather.icon;
      city = cachedWeather.city;
      useCache = true;
    } else {
      // Cache too old, show no data message
      display.setCursor(0, 20);
      display.setTextSize(1);
      display.println("No weather");
      display.println("data available");
      display.setCursor(0, 40);
      display.println("Check Chronos");
      display.println("app settings");
      return;
    }
  } else {
    // No data available
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println("No weather");
    display.println("data available");
    display.setCursor(0, 40);
    display.println("Check Chronos");
    display.println("app settings");
    return;
  }
  
  // Display weather data (we've already validated it exists above)
  updateScrollingText(city, 20);
    
    // 1. Location - Top header bar (Y: 0-10)
    display.fillRect(0, 0, 128, 11, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(2, 2);
    display.setTextSize(1);
    if (city.length() > 20) {
      String displayText = city.substring(scrollPosition, scrollPosition + 20);
      display.print(displayText);
    } else {
      // Center short city names
      int cityWidth = city.length() * 6;
      int cityX = (128 - cityWidth) / 2;
      display.setCursor(cityX, 2);
      display.print(city);
    }
    display.setTextColor(SSD1306_WHITE);
    
    // Main content area (Y: 11-51, 41px height available)
    // Split: 40% left (51px) for icon + weather text, 60% right (77px) for temp, H, L
    
    // LEFT 40% (0-51px): Icon + Weather text
    int leftAreaWidth = 51; // 40% of 128
    int iconSize = 24;
    int iconX = (leftAreaWidth - iconSize) / 2;
    drawWeatherIcon(weather.icon, iconX, 13);
    
    // Weather description text - below icon, centered
    String weatherDesc = getWeatherDescription(weather.icon);
    int descWidth = weatherDesc.length() * 6;
    int descX = (leftAreaWidth - descWidth) / 2;
    display.setCursor(descX, 40);
    display.setTextSize(1);
    display.print(weatherDesc);
    
    // RIGHT 60% (52-128px): Current temp (centered), H, L
    int rightStartX = 52; // Start of right area (40% of 128)
    int rightAreaWidth = 76; // 60% of 128 (128 - 52)
    int rightCenterX = rightStartX + (rightAreaWidth / 2); // Center of right area
    
    // Current temperature - Centered in right area, at top
    display.setTextSize(2);
    // Calculate width: size 2 is ~12px per digit, size 1 is ~6px per char
    int tempDigits = weather.temp < 10 ? 1 : 2;
    int tempWidth = tempDigits * 12 + 6; // Temp digits (size 2) + "C" (size 1)
    int tempX = rightCenterX - (tempWidth / 2);
    display.setCursor(tempX, 18);
    display.setTextSize(2);
    display.print(weather.temp);
    display.setTextSize(1);
    display.print("C");
    
    // High/Low temperatures - Below temp, centered in right area
    String highStr = "H:" + String(weather.high) + "C";
    String lowStr = "L:" + String(weather.low) + "C";
    int highWidth = highStr.length() * 6;
    int lowWidth = lowStr.length() * 6;
    int spacing = 10; // Space between H and L
    int totalWidth = highWidth + spacing + lowWidth;
    int highX = rightCenterX - (totalWidth / 2);
    int lowX = highX + highWidth + spacing;
    
    display.setCursor(highX, 40);
    display.setTextSize(1);
    display.print("H:");
    display.print(weather.high);
    display.print("C");
    
    display.setCursor(lowX, 40);
    display.setTextSize(1);
    display.print("L:");
    display.print(weather.low);
    display.print("C");
    
    // 6. Time (DD/MM hh:mm) - Bottom bar, moved up 2px (Y: 52-59)
    display.fillRect(0, 52, 128, 7, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    // Center the time text
    String timeStr = "";
    int day = rtc.getDay();
    int month = rtc.getMonth();
    int hour = rtc.getHour();
    int minute = rtc.getMinute();
    if (day < 10) timeStr += "0";
    timeStr += String(day);
    timeStr += "/";
    if (month < 10) timeStr += "0";
    timeStr += String(month);
    timeStr += " ";
    if (hour < 10) timeStr += "0";
    timeStr += String(hour);
    timeStr += ":";
    if (minute < 10) timeStr += "0";
    timeStr += String(minute);
    
    int timeWidth = timeStr.length() * 6;
    int timeX = (128 - timeWidth) / 2;
    display.setCursor(timeX, 54);
    display.setTextSize(1);
    display.print(timeStr);
    
    display.setTextColor(SSD1306_WHITE);
}

void displayNotification() {
  if (notificationQueueCount == 0) {
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println("No notification");
    return;
  }

  // Get current notification from queue
  Notification currentNotification = notificationQueue[currentNotificationIndex];

  // 1. Notification number at top (e.g., "1/4")
  display.setTextSize(1);
  String numberStr = String(currentNotificationIndex + 1) + "/" + String(notificationQueueCount);
  int numberWidth = numberStr.length() * 6;
  int numberX = (128 - numberWidth) / 2;
  display.setCursor(numberX, 2);
  display.print(numberStr);
  
  // Top separator line
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  
  // 2. App name (centered, below number)
  String appName = currentNotification.app;
  if (appName.length() == 0) {
    appName = "Unknown App";
  }
  int appWidth = appName.length() * 6;
  int appX = (128 - appWidth) / 2;
  display.setCursor(appX, 14);
  display.setTextSize(1);
  display.print(appName);
  
  // Middle separator line
  display.drawLine(0, 24, 127, 24, SSD1306_WHITE);
  
  // 3. Content (message) as 3 lines with word wrapping and ellipsis
  String content = currentNotification.message;
  if (content.length() == 0) {
    content = currentNotification.title; // Fallback to title if no message
  }
  if (content.length() == 0) {
    content = "No content";
  }
  
  // Content area: Y: 28-58 (30px for 3 lines at 10px per line)
  int contentStartY = 28;
  int lineHeight = 10;
  int maxLines = 3;
  int contentAreaWidth = 128 - 4; // Leave 2px margin on each side
  int maxCharsPerLine = contentAreaWidth / 6; // ~20 chars per line
  
  // Word wrap and display content across 3 lines
  int currentPos = 0;
  int linesDisplayed = 0;
  int currentY = contentStartY;
  
  while (currentPos < (int)content.length() && linesDisplayed < maxLines) {
    // Calculate how many characters fit on this line
    int charsToShow = min(maxCharsPerLine, (int)content.length() - currentPos);
    String line = content.substring(currentPos, currentPos + charsToShow);
    
    // If not at the end and not the last line, try to break at word boundary
    if (currentPos + charsToShow < (int)content.length() && linesDisplayed < maxLines - 1) {
      int lastSpace = line.lastIndexOf(' ');
      if (lastSpace > 0 && lastSpace > maxCharsPerLine / 2) {
        line = line.substring(0, lastSpace);
        charsToShow = lastSpace + 1;
      }
    }
    
    // Check if this is the last line and we need ellipsis
    bool isLastLine = (linesDisplayed == maxLines - 1);
    bool hasMoreContent = (currentPos + charsToShow < (int)content.length());
    
    if (isLastLine && hasMoreContent) {
      // Truncate and add ellipsis if needed
      int ellipsisChars = 3; // "..."
      int availableChars = maxCharsPerLine - ellipsisChars;
      if (line.length() > availableChars) {
        line = line.substring(0, availableChars);
        charsToShow = availableChars;
      }
      line += "...";
    }
    
    display.setCursor(2, currentY);
    display.print(line);
    
    currentY += lineHeight;
    linesDisplayed++;
    currentPos += charsToShow;
    
    // If we added ellipsis, stop processing
    if (isLastLine && hasMoreContent) {
      break;
    }
  }
  
  // Bottom separator line
  display.drawLine(0, 63, 127, 63, SSD1306_WHITE);
}

// Helper function to draw complete navigation arrow
void drawNavigationArrow(String direction, int x, int y) {
  direction.toLowerCase();
  int size = 40; // Larger arrow for better visibility
  
  if (direction.indexOf("left") >= 0 || direction.indexOf("turn left") >= 0) {
    // Complete left arrow (←)
    int centerX = x + size/2;
    int centerY = y + size/2;
    // Arrow head (pointing left)
    display.fillTriangle(x, centerY, centerX - 8, centerY - 12, centerX - 8, centerY - 4, SSD1306_WHITE);
    display.fillTriangle(x, centerY, centerX - 8, centerY + 12, centerX - 8, centerY + 4, SSD1306_WHITE);
    // Arrow shaft
    display.fillRect(centerX - 8, centerY - 6, size/2, 12, SSD1306_WHITE);
  } else if (direction.indexOf("right") >= 0 || direction.indexOf("turn right") >= 0) {
    // Complete right arrow (→)
    int centerX = x + size/2;
    int centerY = y + size/2;
    // Arrow head (pointing right)
    display.fillTriangle(x + size, centerY, centerX + 8, centerY - 12, centerX + 8, centerY - 4, SSD1306_WHITE);
    display.fillTriangle(x + size, centerY, centerX + 8, centerY + 12, centerX + 8, centerY + 4, SSD1306_WHITE);
    // Arrow shaft
    display.fillRect(centerX, centerY - 6, size/2, 12, SSD1306_WHITE);
  } else if (direction.indexOf("u-turn") >= 0 || direction.indexOf("uturn") >= 0 || direction.indexOf("turn around") >= 0) {
    // Complete U-turn arrow
    int centerX = x + size/2;
    int centerY = y + size/2;
    // U shape
    display.drawCircle(centerX, centerY, size/3, SSD1306_WHITE);
    display.drawCircle(centerX, centerY, size/3 - 1, SSD1306_WHITE);
    // Arrow pointing up from U
    display.fillTriangle(centerX, y, centerX - 6, y + 8, centerX + 6, y + 8, SSD1306_WHITE);
  } else if (direction.indexOf("slight left") >= 0) {
    // Slight left arrow
    int centerX = x + size/2;
    int centerY = y + size/2;
    // Curved arrow pointing slightly left
    display.fillTriangle(x + 5, centerY, centerX - 5, centerY - 8, centerX - 5, centerY - 2, SSD1306_WHITE);
    display.fillRect(centerX - 5, centerY - 4, size/2 - 5, 8, SSD1306_WHITE);
  } else if (direction.indexOf("slight right") >= 0) {
    // Slight right arrow
    int centerX = x + size/2;
    int centerY = y + size/2;
    // Curved arrow pointing slightly right
    display.fillTriangle(x + size - 5, centerY, centerX + 5, centerY - 8, centerX + 5, centerY - 2, SSD1306_WHITE);
    display.fillRect(centerX, centerY - 4, size/2 - 5, 8, SSD1306_WHITE);
  } else {
    // Complete straight/up arrow (↑)
    int centerX = x + size/2;
    int centerY = y + size/2;
    // Arrow head (pointing up)
    display.fillTriangle(centerX, y, centerX - 12, centerY - 8, centerX - 4, centerY - 8, SSD1306_WHITE);
    display.fillTriangle(centerX, y, centerX + 12, centerY - 8, centerX + 4, centerY - 8, SSD1306_WHITE);
    // Arrow shaft
    display.fillRect(centerX - 6, centerY - 8, 12, size/2, SSD1306_WHITE);
  }
}

void displayNavigation() {
  Navigation nav = chronos.getNavigation();
  
  if (nav.active) {
    // Get direction text for arrow
    String dirText = nav.directions;
    if (dirText.length() == 0) {
      dirText = nav.title;
    }
    
    // Split screen: Arrow on left (0-51, ~40%), Text on right (52-127, ~60%)
    // Vertical divider line
    display.drawLine(51, 0, 51, 63, SSD1306_WHITE);
    
    // LEFT SIDE (0-51): Arrow centered vertically
    // Arrow size: 40x40, centered in left area
    int arrowX = (51 - 40) / 2; // Center in left 51 pixels
    int arrowY = (64 - 40) / 2; // Center vertically
    drawNavigationArrow(dirText, arrowX, arrowY);
    
    // RIGHT SIDE (52-127): Text information - use full height with word wrapping
    int rightStartX = 55; // Start text a bit after divider
    int rightWidth = 128 - rightStartX; // Available width for text (~73 pixels)
    int lineHeight = 8; // Height per line (text size 1)
    int currentY = 0; // Track current Y position
    
    // Instruction text - can wrap to multiple lines
    if (dirText.length() > 0) {
      display.setTextSize(1);
      String instruction = dirText;
      instruction.toUpperCase();
      // Clean up instruction text
      instruction.replace("TURN ", "");
      instruction.replace(" IN ", " ");
      
      // Word wrap the instruction text
      int maxCharsPerLine = rightWidth / 6; // ~12 chars per line
      int currentPos = 0;
      
      while (currentPos < (int)instruction.length() && currentY < 64 - lineHeight) {
        int charsToShow = min(maxCharsPerLine, (int)instruction.length() - currentPos);
        String line = instruction.substring(currentPos, currentPos + charsToShow);
        
        // Try to break at word boundary
        if (currentPos + charsToShow < (int)instruction.length()) {
          int lastSpace = line.lastIndexOf(' ');
          if (lastSpace > 0) {
            line = line.substring(0, lastSpace);
            charsToShow = lastSpace + 1;
          }
        }
        
        display.setCursor(rightStartX, currentY);
        display.print(line);
        currentY += lineHeight;
        currentPos += charsToShow;
      }
      currentY += 2; // Small spacing after instruction
    }
    
    // Distance to next turn - prominent, can wrap
    if (nav.title.length() > 0 && currentY < 64 - 16) {
      display.setCursor(rightStartX, currentY);
      display.setTextSize(2);
      String dist = nav.title;
      dist.trim();
      // Truncate if too long for one line (size 2 = ~6 chars max)
      int maxDistChars = rightWidth / 12;
      if (dist.length() > maxDistChars) {
        dist = dist.substring(0, maxDistChars);
      }
      display.print(dist);
      currentY += 18; // Size 2 text height + spacing
    }
    
    // Total distance - can wrap to multiple lines
    if (nav.distance.length() > 0 && currentY < 64 - lineHeight) {
      display.setTextSize(1);
      String dist = nav.distance;
      int maxChars = rightWidth / 6;
      
      int currentPos = 0;
      while (currentPos < (int)dist.length() && currentY < 64 - lineHeight) {
        int charsToShow = min(maxChars, (int)dist.length() - currentPos);
        String line = dist.substring(currentPos, currentPos + charsToShow);
        
        // Try to break at word boundary
        if (currentPos + charsToShow < (int)dist.length()) {
          int lastSpace = line.lastIndexOf(' ');
          if (lastSpace > 0) {
            line = line.substring(0, lastSpace);
            charsToShow = lastSpace + 1;
          }
        }
        
        display.setCursor(rightStartX, currentY);
        display.print(line);
        currentY += lineHeight;
        currentPos += charsToShow;
      }
    }
    
    // Duration - can wrap to multiple lines
    if (nav.duration.length() > 0 && currentY < 64 - lineHeight) {
      display.setTextSize(1);
      String dur = nav.duration;
      int maxChars = rightWidth / 6;
      
      int currentPos = 0;
      while (currentPos < (int)dur.length() && currentY < 64 - lineHeight) {
        int charsToShow = min(maxChars, (int)dur.length() - currentPos);
        String line = dur.substring(currentPos, currentPos + charsToShow);
        
        // Try to break at word boundary
        if (currentPos + charsToShow < (int)dur.length()) {
          int lastSpace = line.lastIndexOf(' ');
          if (lastSpace > 0) {
            line = line.substring(0, lastSpace);
            charsToShow = lastSpace + 1;
          }
        }
        
        display.setCursor(rightStartX, currentY);
        display.print(line);
        currentY += lineHeight;
        currentPos += charsToShow;
      }
    }
  } else {
    // No navigation - centered message
    display.setCursor(20, 25);
    display.setTextSize(1);
    display.println("No navigation");
    display.setCursor(30, 35);
    display.println("active");
  }
}
