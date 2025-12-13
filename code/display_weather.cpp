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
  // Rain: 500-531 (light to heavy rain)
  // Thunderstorm: 200-232
  // Snow: 600-622 (light to heavy snow)
  // Mist/fog: 701-781
  // Drizzle: 300-399
  // Extreme: 900-962
  
  // Icon size: 36x36 (height x width) - Pixel art style for monochrome OLED
  // Center point: x+18 (width center), y+18 (height center)
  
  if (icon == 800) {
    // Clear sky - clean sun with 8 rays (pixel perfect)
    display.fillCircle(x + 18, y + 18, 10, SSD1306_WHITE);
    // 8 rays in cardinal and diagonal directions
    display.fillRect(x + 18, y + 2, 2, 6, SSD1306_WHITE); // Top
    display.fillRect(x + 18, y + 28, 2, 6, SSD1306_WHITE); // Bottom
    display.fillRect(x + 2, y + 18, 6, 2, SSD1306_WHITE); // Left
    display.fillRect(x + 28, y + 18, 6, 2, SSD1306_WHITE); // Right
    // Diagonal rays (2x2 squares)
    display.fillRect(x + 5, y + 5, 2, 2, SSD1306_WHITE); // Top-left
    display.fillRect(x + 29, y + 29, 2, 2, SSD1306_WHITE); // Bottom-right
    display.fillRect(x + 29, y + 5, 2, 2, SSD1306_WHITE); // Top-right
    display.fillRect(x + 5, y + 29, 2, 2, SSD1306_WHITE); // Bottom-left
  } else if (icon == 801) {
    // Few clouds - sun peeking behind small cloud
    // Sun (top-left area)
    display.fillCircle(x + 9, y + 9, 7, SSD1306_WHITE);
    display.fillRect(x + 9, y + 2, 2, 4, SSD1306_WHITE); // Top ray
    display.fillRect(x + 2, y + 9, 4, 2, SSD1306_WHITE); // Left ray
    // Cloud (bottom-right, overlapping sun slightly)
    display.fillCircle(x + 20, y + 20, 6, SSD1306_WHITE);
    display.fillCircle(x + 27, y + 21, 5, SSD1306_WHITE);
    display.fillRect(x + 19, y + 21, 9, 5, SSD1306_WHITE);
  } else if (icon >= 200 && icon < 300) {
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
  } else if (icon >= 300 && icon < 400) {
    // Drizzle - cloud with light rain
    // Cloud
    display.fillCircle(x + 10, y + 8, 5, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 8, 6, SSD1306_WHITE);
    display.fillRect(x + 9, y + 10, 11, 4, SSD1306_WHITE);
    // Light drizzle (thin lines)
    display.drawPixel(x + 13, y + 17, SSD1306_WHITE);
    display.drawPixel(x + 13, y + 19, SSD1306_WHITE);
    display.drawPixel(x + 18, y + 18, SSD1306_WHITE);
    display.drawPixel(x + 18, y + 20, SSD1306_WHITE);
    display.drawPixel(x + 23, y + 17, SSD1306_WHITE);
    display.drawPixel(x + 23, y + 19, SSD1306_WHITE);
  } else if (icon >= 500 && icon < 600) {
    // Rain - cloud with rain drops
    // Cloud
    display.fillCircle(x + 9, y + 7, 6, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 7, 7, SSD1306_WHITE);
    display.fillCircle(x + 26, y + 8, 5, SSD1306_WHITE);
    display.fillRect(x + 8, y + 9, 20, 5, SSD1306_WHITE);
    // Rain drops (vertical lines, staggered)
    display.fillRect(x + 12, y + 16, 1, 8, SSD1306_WHITE);
    display.fillRect(x + 16, y + 17, 1, 9, SSD1306_WHITE);
    display.fillRect(x + 20, y + 16, 1, 8, SSD1306_WHITE);
    display.fillRect(x + 24, y + 17, 1, 9, SSD1306_WHITE);
  } else if (icon >= 600 && icon < 700) {
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
  } else if (icon >= 700 && icon < 800) {
    // Mist/fog - horizontal wavy lines
    display.drawLine(x + 3, y + 9, x + 33, y + 11, SSD1306_WHITE);
    display.drawLine(x + 4, y + 13, x + 32, y + 14, SSD1306_WHITE);
    display.drawLine(x + 3, y + 17, x + 33, y + 18, SSD1306_WHITE);
    display.drawLine(x + 5, y + 21, x + 31, y + 22, SSD1306_WHITE);
    display.drawLine(x + 4, y + 25, x + 32, y + 26, SSD1306_WHITE);
  } else if (icon == 802) {
    // Scattered clouds - sun with partial cloud cover
    // Sun (visible on left)
    display.fillCircle(x + 8, y + 9, 6, SSD1306_WHITE);
    display.fillRect(x + 8, y + 3, 2, 3, SSD1306_WHITE); // Top ray
    display.fillRect(x + 2, y + 9, 3, 2, SSD1306_WHITE); // Left ray
    // Clouds (covering right side)
    display.fillCircle(x + 17, y + 10, 6, SSD1306_WHITE);
    display.fillCircle(x + 24, y + 10, 6, SSD1306_WHITE);
    display.fillRect(x + 16, y + 11, 11, 5, SSD1306_WHITE);
  } else if (icon == 803) {
    // Broken clouds - mostly cloudy with small sun visible
    // Small sun (top-left)
    display.fillCircle(x + 6, y + 6, 5, SSD1306_WHITE);
    display.fillRect(x + 6, y + 1, 2, 3, SSD1306_WHITE);
    display.fillRect(x + 1, y + 6, 3, 2, SSD1306_WHITE);
    // Larger clouds
    display.fillCircle(x + 16, y + 9, 7, SSD1306_WHITE);
    display.fillCircle(x + 25, y + 9, 7, SSD1306_WHITE);
    display.fillCircle(x + 20, y + 12, 5, SSD1306_WHITE);
    display.fillRect(x + 15, y + 11, 16, 6, SSD1306_WHITE);
  } else if (icon == 804) {
    // Overcast - full cloud cover (no sun visible)
    display.fillCircle(x + 9, y + 8, 7, SSD1306_WHITE);
    display.fillCircle(x + 18, y + 8, 8, SSD1306_WHITE);
    display.fillCircle(x + 27, y + 9, 6, SSD1306_WHITE);
    display.fillCircle(x + 13, y + 11, 5, SSD1306_WHITE);
    display.fillRect(x + 8, y + 10, 22, 8, SSD1306_WHITE);
  } else {
    // Default - generic cloud
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
  int iconWidth = 36; // Icon width
  int iconHeight = 36; // Icon height
  int iconX = (leftAreaWidth - iconWidth) / 2;
  drawWeatherIcon(weather.icon, iconX, 16);
  
  // Weather description text - below icon, centered
  String weatherDesc = getWeatherDescription(weather.icon);
  int descWidth = weatherDesc.length() * 6;
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
  String uvStr = "UV:" + String(weather.uv);
  // Pressure truncated to 3 characters
  String pressureStr = String(weather.pressure);
  if (pressureStr.length() > 3) {
    pressureStr = pressureStr.substring(0, 3);
  }
  String pStr = "P:" + pressureStr;
  
  int uvWidth = uvStr.length() * 6;
  int pWidth = pStr.length() * 6;
  int uvPspacing = 10; // Space between UV and P
  int uvPtotalWidth = uvWidth + uvPspacing + pWidth;
  int uvX = rightCenterX - (uvPtotalWidth / 2);
  int pX = uvX + uvWidth + uvPspacing;
  
  display.setCursor(uvX, 34);
  display.print(uvStr);
  
  display.setCursor(pX, 34);
  display.print(pStr);
  
  // High/Low temperatures - Below UV/P, centered in right area
  String highStr = "H:" + String(weather.high) + "C";
  String lowStr = "L:" + String(weather.low) + "C";
  int highWidth = highStr.length() * 6;
  int lowWidth = lowStr.length() * 6;
  int spacing = 10; // Space between H and L
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
  display.setCursor(timeX, 56);
  display.setTextSize(1);
  display.print(timeStr);
  
  display.setTextColor(SSD1306_WHITE);
}

