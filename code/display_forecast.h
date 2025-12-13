/*
 * Display Forecast - Weather forecast display functions
 */

#ifndef DISPLAY_FORECAST_H
#define DISPLAY_FORECAST_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>

// Forward declarations
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;

// Function declarations
void displayForecast();

#endif // DISPLAY_FORECAST_H

