/*
 * Display Weather - Weather display functions, icons, and scrolling
 */

#ifndef DISPLAY_WEATHER_H
#define DISPLAY_WEATHER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>
#include <ESP32Time.h>

// Forward declarations
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;

// Function declarations
void displayWeather();
String getWeatherDescription(int icon);
void drawWeatherIcon(int icon, int x, int y);
void updateScrollingText(String text, int maxWidth);

#endif // DISPLAY_WEATHER_H

