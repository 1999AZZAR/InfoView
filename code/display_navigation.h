/*
 * Display Navigation - Navigation display functions
 */

#ifndef DISPLAY_NAVIGATION_H
#define DISPLAY_NAVIGATION_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>

// Forward declarations
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;

// Function declarations
void displayNavigation();
void drawNavigationArrow(String direction, int x, int y);

#endif // DISPLAY_NAVIGATION_H

