/*
 * Weather Cache - Manages local weather data caching for offline operation
 */

#include "weather_cache.h"
#include "config.h"

// Forward declaration
extern ChronosESP32 chronos;

// Weather cache
CachedWeather cachedWeather;

void initWeatherCache() {
  cachedWeather.valid = false;
  cachedWeather.temp = 0;
  cachedWeather.high = 0;
  cachedWeather.low = 0;
  cachedWeather.icon = 0;
  cachedWeather.city = "";
  cachedWeather.timestamp = 0;
}

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

bool getCachedWeather(Weather& weather, String& city) {
  if (cachedWeather.valid) {
    unsigned long age = millis() - cachedWeather.timestamp;
    if (age < CACHE_MAX_AGE) {
      weather.temp = cachedWeather.temp;
      weather.high = cachedWeather.high;
      weather.low = cachedWeather.low;
      weather.icon = cachedWeather.icon;
      city = cachedWeather.city;
      return true;
    }
  }
  return false;
}

