/*
 * Display Time - Time display functions
 */

#ifndef DISPLAY_TIME_H
#define DISPLAY_TIME_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>
#include <ESP32Time.h>

// Forward declarations
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;

// Function declarations
void displayTime();

#endif // DISPLAY_TIME_H

