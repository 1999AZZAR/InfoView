/*
 * Display Manager - Handles display mode switching and updates
 */

#include "display_manager.h"
#include "display_time.h"
#include "display_weather.h"
#include "display_forecast.h"
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

// Helper function to get the first enabled face (default fallback)
static DisplayMode getFirstEnabledFace() {
  if (ENABLE_TIME_FACE) return MODE_TIME;
  if (ENABLE_WEATHER_FACE) return MODE_WEATHER;
  if (ENABLE_FORECAST_FACE) return MODE_FORECAST;
  // If all are disabled, default to time (shouldn't happen in normal use)
  return MODE_TIME;
}

// Helper function to get next enabled face in the cycle
static DisplayMode getNextEnabledFace(DisplayMode current) {
  // Cycle: TIME -> WEATHER -> FORECAST -> TIME
  if (current == MODE_TIME) {
    if (ENABLE_WEATHER_FACE && hasWeatherData()) {
      return MODE_WEATHER;
    } else if (ENABLE_FORECAST_FACE && chronos.getWeatherCount() >= 2) {
      return MODE_FORECAST;
    } else {
      return MODE_TIME; // Stay on time if others disabled or no data
    }
  } else if (current == MODE_WEATHER) {
    if (ENABLE_FORECAST_FACE && chronos.getWeatherCount() >= 2) {
      return MODE_FORECAST;
    } else if (ENABLE_TIME_FACE) {
      return MODE_TIME;
    } else {
      return MODE_WEATHER; // Stay on weather if time disabled
    }
  } else if (current == MODE_FORECAST) {
    if (ENABLE_TIME_FACE) {
      return MODE_TIME;
    } else if (ENABLE_WEATHER_FACE && hasWeatherData()) {
      return MODE_WEATHER;
    } else {
      return MODE_FORECAST; // Stay on forecast if others disabled
    }
  }
  return getFirstEnabledFace();
}

void initDisplayManager() {
  // Initialize to first enabled face
  currentMode = getFirstEnabledFace();
  previousMode = currentMode;
  lastModeSwitch = 0;
  lastDisplayUpdate = 0;
  displayNeedsUpdate = true;
}

void updateDisplay() {
  unsigned long currentTime = millis();
  bool modeChanged = false;
  // Cache navigation state to avoid multiple calls
  static Navigation cachedNav;
  static unsigned long lastNavCheck = 0;
  // Update navigation cache every 100ms (navigation updates frequently)
  if (currentTime - lastNavCheck >= 100) {
    cachedNav = chronos.getNavigation();
    lastNavCheck = currentTime;
  }
  Navigation nav = cachedNav;

  // Priority 1: Notifications (highest priority, interrupts everything)
  if (ENABLE_NOTIFICATION_FACE && hasActiveNotification(currentTime, nav.active)) {
    if (currentMode != MODE_NOTIFICATION) {
      currentMode = MODE_NOTIFICATION;
      modeChanged = true;
      displayNeedsUpdate = true;
    }
  } 
  // Priority 2: Navigation (overrides time/weather loop immediately)
  else if (ENABLE_NAVIGATION_FACE && chronos.isConnected() && nav.active) {
    if (currentMode != MODE_NAVIGATION) {
      currentMode = MODE_NAVIGATION;
      modeChanged = true;
      lastModeSwitch = currentTime; // Reset timer when entering navigation
      displayNeedsUpdate = true;
    }
  } 
  // Priority 3: Time/Weather loop (normal operation)
  else {
    // If navigation was just deactivated, switch to first enabled face
    if (currentMode == MODE_NAVIGATION) {
      currentMode = getFirstEnabledFace();
      modeChanged = true;
      lastModeSwitch = currentTime;
      displayNeedsUpdate = true;
    }
    // Switch between enabled faces with different durations
    // Cycle: TIME (20s) -> WEATHER (10s) -> FORECAST (10s) -> TIME
    unsigned long modeDuration = 0;
    if (currentMode == MODE_TIME) {
      modeDuration = MODE_TIME_DURATION;
    } else if (currentMode == MODE_WEATHER) {
      modeDuration = MODE_WEATHER_DURATION;
    } else if (currentMode == MODE_FORECAST) {
      modeDuration = MODE_FORECAST_DURATION;
    }
    
    if (modeDuration > 0 && (currentTime - lastModeSwitch) >= modeDuration) {
      DisplayMode nextMode = getNextEnabledFace(currentMode);
      if (nextMode != currentMode) {
        currentMode = nextMode;
        modeChanged = true;
        lastModeSwitch = currentTime;
        displayNeedsUpdate = true;
      } else {
        // No other enabled face available, stay on current
        lastModeSwitch = currentTime; // Reset timer to avoid constant checking
      }
    }
    
    // Process notification queue (handles timeout and cleanup)
    if (ENABLE_NOTIFICATION_FACE) {
      processNotificationQueue(currentTime, nav.active);
    }
    
    // If notifications expired and queue is empty, switch to first enabled face
    if (notificationQueueCount == 0 && currentMode == MODE_NOTIFICATION) {
      currentMode = getFirstEnabledFace();
      modeChanged = true;
      displayNeedsUpdate = true;
    }
  }

  // Safety check: if current mode is disabled, switch to first enabled face
  bool currentModeEnabled = false;
  switch (currentMode) {
    case MODE_TIME:
      currentModeEnabled = ENABLE_TIME_FACE;
      break;
    case MODE_WEATHER:
      currentModeEnabled = ENABLE_WEATHER_FACE;
      break;
    case MODE_FORECAST:
      currentModeEnabled = ENABLE_FORECAST_FACE;
      break;
    case MODE_NOTIFICATION:
      currentModeEnabled = ENABLE_NOTIFICATION_FACE;
      break;
    case MODE_NAVIGATION:
      currentModeEnabled = ENABLE_NAVIGATION_FACE;
      break;
  }
  
  if (!currentModeEnabled) {
    currentMode = getFirstEnabledFace();
    modeChanged = true;
    displayNeedsUpdate = true;
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
    // Smooth transition: brief dim effect when mode changes
    if (modeChanged && previousMode != MODE_TIME) {
      // Very brief dim for smooth transition (only for non-time modes)
      display.dim(true);
      delay(5); // Minimal delay for smooth transition
      display.dim(false);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    switch (currentMode) {
      case MODE_TIME:
        if (ENABLE_TIME_FACE) {
          displayTime();
        }
        break;
      case MODE_WEATHER:
        if (ENABLE_WEATHER_FACE) {
          displayWeather();
        }
        break;
      case MODE_FORECAST:
        if (ENABLE_FORECAST_FACE) {
          displayForecast();
        }
        break;
      case MODE_NOTIFICATION:
        if (ENABLE_NOTIFICATION_FACE) {
          displayNotification();
        }
        break;
      case MODE_NAVIGATION:
        if (ENABLE_NAVIGATION_FACE) {
          displayNavigation();
        }
        break;
    }

    display.display();
    previousMode = currentMode;
    lastDisplayUpdate = currentTime;
    displayNeedsUpdate = false;
  }
}

