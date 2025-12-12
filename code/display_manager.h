/*
 * Display Manager - Handles display mode switching and updates
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>
#include <ESP32Time.h>

// Display modes
enum DisplayMode {
  MODE_TIME,
  MODE_WEATHER,
  MODE_NOTIFICATION,
  MODE_NAVIGATION
};

// Forward declarations
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;

// Display state
extern DisplayMode currentMode;
extern DisplayMode previousMode;
extern unsigned long lastModeSwitch;
extern unsigned long lastDisplayUpdate;
extern bool displayNeedsUpdate;

// Function declarations
void initDisplayManager();
void updateDisplay();

#endif // DISPLAY_MANAGER_H

