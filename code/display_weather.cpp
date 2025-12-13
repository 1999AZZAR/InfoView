/*
 * Display Weather - Weather display functions, icons, and scrolling
 */

#include "display_weather.h"
#include "weather_cache.h"
#include "config.h"
#include <string.h>

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
  // ChronosESP32 uses icon codes 0-9 only
  switch (icon) {
    case 0:
      return "Clear";
    case 1:
      return "Sunny";
    case 2:
      return "Cloudy";
    case 3:
      return "Rain";
    case 4:
      return "Rain";
    case 5:
      return "Storm";
    case 6:
      return "Snow";
    case 7:
      return "Fog";
    case 8:
      return "Drizzle";
    case 9:
      return "Cloudy";
    default:
      // Default to Cloudy for invalid codes
      return "Cloudy";
  }
}

void drawWeatherIcon(int icon, int x, int y) {
  // ChronosESP32 uses icon codes 0-9 only
  // Icon size: 36x36 (height x width) - Pixel art style for monochrome OLED
  // Center point: x+18 (width center), y+18 (height center)
  
  // Get current hour to determine day/night (6 AM - 6 PM = day, 6 PM - 6 AM = night)
  int hour = chronos.getHourC();
  bool isDay = (hour >= 6 && hour < 18);
  
  // Handle ChronosESP32 icon codes (0-9)
  if (icon == 0) {
    // Clear - Show sun during day, moon and stars at night
    int centerX = x + 18;
    int centerY = y + 18;
    
    if (isDay) {
      // Day: Clear/Sunny - Redesigned with better visual appeal
      // Main sun circle (solid, prominent)
      display.fillCircle(centerX, centerY, 10, SSD1306_WHITE);
      
      // Inner highlight circle for depth (creates nice contrast)
      display.drawCircle(centerX, centerY, 7, SSD1306_BLACK);
      display.drawCircle(centerX, centerY, 6, SSD1306_BLACK);
      
      // 8 rays in cardinal and diagonal directions (well-proportioned)
      // Cardinal directions (vertical and horizontal)
      display.fillRect(centerX - 1, y + 2, 2, 4, SSD1306_WHITE); // Top
      display.fillRect(centerX - 1, y + 30, 2, 4, SSD1306_WHITE); // Bottom
      display.fillRect(x + 2, centerY - 1, 4, 2, SSD1306_WHITE); // Left
      display.fillRect(x + 30, centerY - 1, 4, 2, SSD1306_WHITE); // Right
      
      // Diagonal rays (at 45 degrees, slightly longer)
      display.fillRect(x + 5, y + 5, 2, 2, SSD1306_WHITE); // Top-left
      display.fillRect(x + 29, y + 29, 2, 2, SSD1306_WHITE); // Bottom-right
      display.fillRect(x + 29, y + 5, 2, 2, SSD1306_WHITE); // Top-right
      display.fillRect(x + 5, y + 29, 2, 2, SSD1306_WHITE); // Bottom-left
      
      // Additional small rays for more detail
      display.fillRect(x + 8, y + 3, 1, 1, SSD1306_WHITE); // Top-left small
      display.fillRect(x + 27, y + 3, 1, 1, SSD1306_WHITE); // Top-right small
      display.fillRect(x + 3, y + 8, 1, 1, SSD1306_WHITE); // Left-top small
      display.fillRect(x + 3, y + 27, 1, 1, SSD1306_WHITE); // Left-bottom small
      display.fillRect(x + 32, y + 8, 1, 1, SSD1306_WHITE); // Right-top small
      display.fillRect(x + 32, y + 27, 1, 1, SSD1306_WHITE); // Right-bottom small
      display.fillRect(x + 8, y + 32, 1, 1, SSD1306_WHITE); // Bottom-left small
      display.fillRect(x + 27, y + 32, 1, 1, SSD1306_WHITE); // Bottom-right small
    } else {
      // Night: Clear sky with moon and stars
      // Crescent moon (right side)
      display.fillCircle(centerX + 3, centerY - 2, 8, SSD1306_WHITE);
      display.fillCircle(centerX, centerY - 2, 8, SSD1306_BLACK);
      
      // Stars (small 4-pointed stars scattered)
      // Top-left star
      display.fillRect(x + 6, y + 6, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 5, y + 7, 3, 1, SSD1306_WHITE);
      display.fillRect(x + 6, y + 8, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 7, y + 5, 1, 3, SSD1306_WHITE);
      
      // Top-right star
      display.fillRect(x + 28, y + 8, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 27, y + 9, 3, 1, SSD1306_WHITE);
      display.fillRect(x + 28, y + 10, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 29, y + 7, 1, 3, SSD1306_WHITE);
      
      // Bottom-left star
      display.fillRect(x + 5, y + 28, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 4, y + 29, 3, 1, SSD1306_WHITE);
      display.fillRect(x + 5, y + 30, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 6, y + 27, 1, 3, SSD1306_WHITE);
      
      // Bottom-right star (smaller)
      display.fillRect(x + 30, y + 30, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 29, y + 31, 3, 1, SSD1306_WHITE);
      display.fillRect(x + 30, y + 32, 1, 1, SSD1306_WHITE);
      display.fillRect(x + 31, y + 29, 1, 3, SSD1306_WHITE);
    }
  } else if (icon == 1) {
    // Partly Cloudy/Sunny - sun peeking behind small cloud
    // Sun (top-left area)
    display.fillCircle(x + 9, y + 9, 7, SSD1306_WHITE);
    display.fillRect(x + 9, y + 2, 2, 4, SSD1306_WHITE); // Top ray
    display.fillRect(x + 2, y + 9, 4, 2, SSD1306_WHITE); // Left ray
    // Cloud (bottom-right, overlapping sun slightly)
    display.fillCircle(x + 20, y + 20, 6, SSD1306_WHITE);
    display.fillCircle(x + 27, y + 21, 5, SSD1306_WHITE);
    display.fillRect(x + 19, y + 21, 9, 5, SSD1306_WHITE);
  } else if (icon == 2) {
    // Cloudy - full cloud cover
    display.fillCircle(x + 9, y + 8, 7, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 8, 8, SSD1306_WHITE);
    display.fillCircle(x + 27, y + 9, 6, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 11, 5, SSD1306_WHITE);
    display.fillRect(x + 8, y + 10, 22, 8, SSD1306_WHITE);
  } else if (icon == 3) {
    // Light Rain - cloud with light rain drops
    // Cloud
    display.fillCircle(x + 9, y + 7, 6, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 7, 7, SSD1306_WHITE);
    display.fillCircle(x + 26, y + 8, 5, SSD1306_WHITE);
    display.fillRect(x + 8, y + 9, 20, 5, SSD1306_WHITE);
    // Light rain drops (vertical lines, fewer than heavy rain)
    display.fillRect(x + 13, y + 16, 1, 7, SSD1306_WHITE);
    display.fillRect(x + 18, y + 17, 1, 8, SSD1306_WHITE);
    display.fillRect(x + 23, y + 16, 1, 7, SSD1306_WHITE);
  } else if (icon == 4) {
    // Heavy Rain - cloud with heavy rain drops
    // Cloud
    display.fillCircle(x + 9, y + 7, 6, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 7, 7, SSD1306_WHITE);
    display.fillCircle(x + 26, y + 8, 5, SSD1306_WHITE);
    display.fillRect(x + 8, y + 9, 20, 5, SSD1306_WHITE);
    // Heavy rain drops (vertical lines, staggered)
    display.fillRect(x + 12, y + 16, 1, 8, SSD1306_WHITE);
    display.fillRect(x + 16, y + 17, 1, 9, SSD1306_WHITE);
    display.fillRect(x + 20, y + 16, 1, 8, SSD1306_WHITE);
    display.fillRect(x + 24, y + 17, 1, 9, SSD1306_WHITE);
  } else if (icon == 5) {
    // Thunderstorm - cloud with lightning bolt
    // Cloud (top, dark and full)
    display.fillCircle(x + 10, y + 8, 6, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 7, 7, SSD1306_WHITE);
    display.fillCircle(x + 25, y + 8, 5, SSD1306_WHITE);
    display.fillRect(x + 9, y + 9, 18, 5, SSD1306_WHITE);
    // Lightning bolt (Z-shaped, bold)
    display.fillTriangle(x + 17, y + 14, x + 20, y + 14, x + 18, y + 18, SSD1306_WHITE);
    display.fillTriangle(x + 18, y + 18, x + 14, y + 22, x + 18, y + 22, SSD1306_WHITE);
    display.fillRect(x + 17, y + 22, 3, 12, SSD1306_WHITE);
    display.fillRect(x + 16, y + 28, 2, 5, SSD1306_WHITE);
  } else if (icon == 6) {
    // Snow - cloud with snowflake
    // Cloud
    display.fillCircle(x + 9, y + 7, 6, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 7, 7, SSD1306_WHITE);
    display.fillRect(x + 8, y + 9, 12, 4, SSD1306_WHITE);
    // Snowflake (symmetric 6-arm design, centered)
    int sx = x + 18, sy = y + 21;
    // Main arms (vertical and horizontal)
    display.fillRect(sx - 1, sy - 6, 3, 12, SSD1306_WHITE);
    display.fillRect(sx - 6, sy - 1, 12, 3, SSD1306_WHITE);
    // Diagonal arms (small squares)
    display.fillRect(sx - 5, sy - 5, 2, 2, SSD1306_WHITE);
    display.fillRect(sx + 3, sy + 3, 2, 2, SSD1306_WHITE);
    display.fillRect(sx + 3, sy - 5, 2, 2, SSD1306_WHITE);
    display.fillRect(sx - 5, sy + 3, 2, 2, SSD1306_WHITE);
  } else if (icon == 7) {
    // Mist/fog - horizontal wavy lines
    display.drawLine(x + 3, y + 9, x + 33, y + 11, SSD1306_WHITE);
    display.drawLine(x + 4, y + 13, x + 32, y + 14, SSD1306_WHITE);
    display.drawLine(x + 3, y + 17, x + 33, y + 18, SSD1306_WHITE);
    display.drawLine(x + 5, y + 21, x + 31, y + 22, SSD1306_WHITE);
    display.drawLine(x + 4, y + 25, x + 32, y + 26, SSD1306_WHITE);
  } else if (icon == 8) {
    // Drizzle - cloud with light rain
    // Cloud
    display.fillCircle(x + 10, y + 8, 5, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 8, 6, SSD1306_WHITE);
    display.fillRect(x + 9, y + 10, 11, 4, SSD1306_WHITE);
    // Light drizzle (thin lines/dots)
    display.drawPixel(x + 13, y + 17, SSD1306_WHITE);
    display.drawPixel(x + 13, y + 19, SSD1306_WHITE);
    display.drawPixel(x + 18, y + 18, SSD1306_WHITE);
    display.drawPixel(x + 18, y + 20, SSD1306_WHITE);
    display.drawPixel(x + 23, y + 17, SSD1306_WHITE);
    display.drawPixel(x + 23, y + 19, SSD1306_WHITE);
  } else if (icon == 9) {
    // Overcast/Cloudy - full cloud cover (no sun visible)
    display.fillCircle(x + 9, y + 8, 7, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 8, 8, SSD1306_WHITE);
    display.fillCircle(x + 27, y + 9, 6, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 11, 5, SSD1306_WHITE);
    display.fillRect(x + 8, y + 10, 22, 8, SSD1306_WHITE);
  } else {
    // Default - generic cloud for invalid icon codes (outside 0-9)
    display.fillCircle(x + 9, y + 8, 7, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 8, 8, SSD1306_WHITE);
    display.fillCircle(x + 27, y + 9, 6, SSD1306_WHITE);
    display.fillRect(x + 8, y + 10, 21, 7, SSD1306_WHITE);
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
  String city;
  
  // Try to get current weather data
  if (chronos.getWeatherCount() > 0) {
    weather = chronos.getWeatherAt(0);
    WeatherLocation location = chronos.getWeatherLocation();
    city = location.city.length() > 0 ? location.city : chronos.getWeatherCity();
  } else if (getCachedWeather(weather, city)) {
    // Using cached data
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
  display.setTextSize(1);
  
  // Determine what text to display
  String displayText;
  if (city.length() > 20) {
    // Scrolling text - get visible portion
    displayText = city.substring(scrollPosition, scrollPosition + 20);
  } else {
    // Short city name - use full text
    displayText = city;
  }
  
  // Center the text dynamically (works for both scrolling and static)
  int textWidth = displayText.length() * 6;
  int textX = (SCREEN_WIDTH - textWidth) / 2;
  display.setCursor(textX, 2);
  display.print(displayText);
  
  display.setTextColor(SSD1306_WHITE);
  
  // Main content area (Y: 11-51, 41px height available)
  // Split: 40% left (51px) for icon + weather text, 60% right (77px) for temp, H, L
  
  // LEFT 40% (0-51px): Icon + Weather text
  int leftAreaWidth = 51; // 40% of 128
  int iconWidth = 36; // Icon width
  int iconHeight = 36; // Icon height
  int iconX = (leftAreaWidth - iconWidth) / 2;
  drawWeatherIcon(weather.icon, iconX, 16);
  
  // Weather description text - below icon, centered
  // Get description and calculate width directly
  const char* weatherDesc = getWeatherDescription(weather.icon).c_str();
  int descWidth = strlen(weatherDesc) * 6;
  int descX = (leftAreaWidth - descWidth) / 2;
  display.setCursor(descX, 45);
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
  display.setCursor(tempX, 15);
  display.setTextSize(2);
  display.print(weather.temp);
  display.setTextSize(1);
  display.print("C");
  
  // UV and Pressure - Below temp, above H/L, centered in right area
  display.setTextSize(1);
  // Calculate widths directly without creating String objects
  int uvValue = weather.uv;
  int pressureValue = weather.pressure;
  // Truncate pressure to 3 digits
  if (pressureValue > 999) pressureValue = 999;
  
  // Calculate positions directly
  // UV: "UV:" (3 chars) + 1-2 digits = 4-5 chars total
  int uvDigits = uvValue < 10 ? 1 : 2;
  int uvWidth = (3 + uvDigits) * 6; // "UV:" + digits
  int pWidth = 5 * 6; // "P:" (2) + 3 digits max = 5 chars
  int uvPspacing = 10;
  int uvPtotalWidth = uvWidth + uvPspacing + pWidth;
  int uvX = rightCenterX - (uvPtotalWidth / 2);
  int pX = uvX + uvWidth + uvPspacing;
  
  display.setCursor(uvX, 34);
  display.print("UV:");
  display.print(uvValue);
  
  display.setCursor(pX, 34);
  display.print("P:");
  display.print(pressureValue);
  
  // High/Low temperatures - Below UV/P, centered in right area
  // Calculate widths directly (H: + 1-2 digits + C = 4-5 chars, same for L:)
  int highDigits = weather.high < 10 ? 1 : 2;
  int lowDigits = weather.low < 10 ? 1 : 2;
  int highWidth = (2 + highDigits + 1) * 6; // "H:" + digits + "C"
  int lowWidth = (2 + lowDigits + 1) * 6; // "L:" + digits + "C"
  int spacing = 10;
  int totalWidth = highWidth + spacing + lowWidth;
  int highX = rightCenterX - (totalWidth / 2);
  int lowX = highX + highWidth + spacing;
  
  display.setCursor(highX, 45);
  display.setTextSize(1);
  display.print("H:");
  display.print(weather.high);
  display.print("C");
  
  display.setCursor(lowX, 45);
  display.setTextSize(1);
  display.print("L:");
  display.print(weather.low);
  display.print("C");
  
  // 6. Time (DD/MM hh:mm) - Bottom bar (Y: 53-64)
  display.fillRect(0, 55, SCREEN_WIDTH, 9, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  // Center the time text - format: DD/MM hh:mm = 11 chars = 66px (fixed)
  int day = rtc.getDay();
  int month = rtc.getMonth();
  int hour = rtc.getHour();
  int minute = rtc.getMinute();
  int timeX = (SCREEN_WIDTH - 66) / 2; // Fixed width: 11 chars * 6px = 66px
  display.setCursor(timeX, 56);
  display.setTextSize(1);
  // Print directly without String concatenation
  if (day < 10) display.print("0");
  display.print(day);
  display.print("/");
  if (month < 10) display.print("0");
  display.print(month);
  display.print(" ");
  if (hour < 10) display.print("0");
  display.print(hour);
  display.print(":");
  if (minute < 10) display.print("0");
  display.print(minute);
  
  display.setTextColor(SSD1306_WHITE);
}

