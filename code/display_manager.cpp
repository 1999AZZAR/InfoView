/*
 * Display Manager - Handles display mode switching and updates
 */

#include "display_manager.h"
#include "display_time.h"
#include "display_weather.h"
#include "display_notification.h"
#include "display_navigation.h"
#include "notification_queue.h"
#include "weather_cache.h"
#include "config.h"

// External variables
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;

// Display state
DisplayMode currentMode = MODE_TIME;
DisplayMode previousMode = MODE_TIME;
unsigned long lastModeSwitch = 0;
unsigned long lastDisplayUpdate = 0;
bool displayNeedsUpdate = true;

// External notification queue variables
extern int notificationQueueCount;
extern int currentNotificationIndex;
extern unsigned long notificationStartTime;

void initDisplayManager() {
  currentMode = MODE_TIME;
  previousMode = MODE_TIME;
  lastModeSwitch = 0;
  lastDisplayUpdate = 0;
  displayNeedsUpdate = true;
}

void updateDisplay() {
  unsigned long currentTime = millis();
  bool modeChanged = false;
  Navigation nav = chronos.getNavigation();

  // Priority 1: Notifications (highest priority, interrupts everything)
  if (hasActiveNotification(currentTime, nav.active)) {
    if (currentMode != MODE_NOTIFICATION) {
      currentMode = MODE_NOTIFICATION;
      modeChanged = true;
      displayNeedsUpdate = true;
    }
  } 
  // Priority 2: Navigation (overrides time/weather loop immediately)
  else if (chronos.isConnected() && nav.active) {
    if (currentMode != MODE_NAVIGATION) {
      currentMode = MODE_NAVIGATION;
      modeChanged = true;
      lastModeSwitch = currentTime; // Reset timer when entering navigation
      displayNeedsUpdate = true;
    }
  } 
  // Priority 3: Time/Weather loop (normal operation)
  else {
    // If navigation was just deactivated, immediately switch to time
    if (currentMode == MODE_NAVIGATION) {
      currentMode = MODE_TIME;
      modeChanged = true;
      lastModeSwitch = currentTime;
      displayNeedsUpdate = true;
    }
    // Switch between time and weather every MODE_SWITCH_INTERVAL seconds
    // Only switch to weather if data is available (current or cached)
    else if ((currentTime - lastModeSwitch) >= MODE_SWITCH_INTERVAL) {
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
        // If coming from notification, start with time
        currentMode = MODE_TIME;
        modeChanged = true;
      }
      if (modeChanged) {
        lastModeSwitch = currentTime;
        displayNeedsUpdate = true;
      }
    }
    
    // Process notification queue (handles timeout and cleanup)
    processNotificationQueue(currentTime, nav.active);
    
    // If notifications expired and queue is empty, mode will switch naturally
    if (notificationQueueCount == 0 && currentMode == MODE_NOTIFICATION) {
      currentMode = MODE_TIME;
      modeChanged = true;
      displayNeedsUpdate = true;
    }
  }

  // Only update display if mode changed or content needs refresh
  // For time mode, update every second for seconds counter
  // For navigation mode, update frequently for smooth real-time updates
  // For other modes, update when mode changes or every 500ms
  bool shouldUpdate = displayNeedsUpdate || modeChanged || 
                      (currentMode == MODE_TIME && (currentTime - lastDisplayUpdate >= 1000)) ||
                      (currentMode == MODE_NAVIGATION && (currentTime - lastDisplayUpdate >= 500)) ||
                      (currentMode != MODE_TIME && currentMode != MODE_NAVIGATION && currentMode != previousMode);
  
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

