/*
 * Display Weather - Weather display functions, icons, and scrolling
 */

#include "display_weather.h"
#include "weather_cache.h"
#include "config.h"

// External objects
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;
extern CachedWeather cachedWeather;

// Scrolling text state
String weatherLocationText = "";
unsigned long lastScrollTime = 0;
int scrollPosition = 0;
unsigned long scrollPauseStart = 0;
bool scrollPaused = true;

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
  } else if (getCachedWeather(weather, city)) {
    useCache = true;
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
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.setTextSize(1);
  if (city.length() > 20) {
    String displayText = city.substring(scrollPosition, scrollPosition + 20);
    display.print(displayText);
  } else {
    // Center short city names
    int cityWidth = city.length() * 6;
    int cityX = (SCREEN_WIDTH - cityWidth) / 2;
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
  
  // 6. Time (DD/MM hh:mm) - Bottom bar (Y: 51-58)
  display.fillRect(0, 51, SCREEN_WIDTH, 7, SSD1306_WHITE);
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
  int timeX = (SCREEN_WIDTH - timeWidth) / 2;
  display.setCursor(timeX, 53);
  display.setTextSize(1);
  display.print(timeStr);
  
  display.setTextColor(SSD1306_WHITE);
}

