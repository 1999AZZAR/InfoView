/*
 * Weather Cache - Manages local weather data caching for offline operation
 */

#ifndef WEATHER_CACHE_H
#define WEATHER_CACHE_H

#include <Arduino.h>
#include <ChronosESP32.h>
#include "config.h"

// Cached weather data structure
struct CachedWeather {
  bool valid;
  int temp;
  int high;
  int low;
  int icon;
  String city;
  unsigned long timestamp; // When data was cached
};

// External cache
extern CachedWeather cachedWeather;

// Function declarations
void initWeatherCache();
void updateWeatherCache();
bool hasWeatherData();
bool getCachedWeather(Weather& weather, String& city);

#endif // WEATHER_CACHE_H

