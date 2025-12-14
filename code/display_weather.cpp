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

void drawWeatherIcon(int icon, int x, int y, int size) {
  // ChronosESP32 uses icon codes 0-9 only
  // Icon size: variable (default 36x36, can be up to 40x40) - Pixel art style for monochrome OLED
  // Scale factor for coordinates (based on size relative to 36)
  float scale = size / 36.0f;
  int centerX = x + size / 2;
  int centerY = y + size / 2;
  
  // Helper macro to scale a coordinate
  #define SCALE_COORD(c) ((int)((c) * scale + 0.5f))
  
  // Get current hour to determine day/night (6 AM - 6 PM = day, 6 PM - 6 AM = night)
  int hour = chronos.getHourC();
  bool isDay = (hour >= 6 && hour < 18);
  
  // Handle ChronosESP32 icon codes (0-9)
  if (icon == 0) {
    // Clear - Show sun during day, moon and stars at night
    
    if (isDay) {
      // Day: Clear/Sunny - Scaled for larger icon
      int sunRadius = SCALE_COORD(10);
      int rayOffset = SCALE_COORD(2);
      int rayLength = SCALE_COORD(4);
      // Main sun circle (solid, prominent)
      display.fillCircle(centerX, centerY, sunRadius, SSD1306_WHITE);
      
      // Inner highlight circle for depth (creates nice contrast)
      display.drawCircle(centerX, centerY, SCALE_COORD(7), SSD1306_BLACK);
      display.drawCircle(centerX, centerY, SCALE_COORD(6), SSD1306_BLACK);
      
      // 8 rays in cardinal and diagonal directions (well-proportioned)
      // Cardinal directions (vertical and horizontal)
      display.fillRect(centerX - 1, y + rayOffset, 2, rayLength, SSD1306_WHITE); // Top
      display.fillRect(centerX - 1, y + size - rayOffset - rayLength, 2, rayLength, SSD1306_WHITE); // Bottom
      display.fillRect(x + rayOffset, centerY - 1, rayLength, 2, SSD1306_WHITE); // Left
      display.fillRect(x + size - rayOffset - rayLength, centerY - 1, rayLength, 2, SSD1306_WHITE); // Right
      
      // Diagonal rays (at 45 degrees, slightly longer)
      int diagOffset = SCALE_COORD(5);
      int diagSize = SCALE_COORD(2);
      display.fillRect(x + diagOffset, y + diagOffset, diagSize, diagSize, SSD1306_WHITE); // Top-left
      display.fillRect(x + size - diagOffset - diagSize, y + size - diagOffset - diagSize, diagSize, diagSize, SSD1306_WHITE); // Bottom-right
      display.fillRect(x + size - diagOffset - diagSize, y + diagOffset, diagSize, diagSize, SSD1306_WHITE); // Top-right
      display.fillRect(x + diagOffset, y + size - diagOffset - diagSize, diagSize, diagSize, SSD1306_WHITE); // Bottom-left
      
      // Additional small rays for more detail
      int smallOffset1 = SCALE_COORD(8);
      int smallOffset2 = SCALE_COORD(3);
      int smallOffset3 = SCALE_COORD(27);
      display.fillRect(x + smallOffset1, y + smallOffset2, 1, 1, SSD1306_WHITE); // Top-left small
      display.fillRect(x + smallOffset3, y + smallOffset2, 1, 1, SSD1306_WHITE); // Top-right small
      display.fillRect(x + smallOffset2, y + smallOffset1, 1, 1, SSD1306_WHITE); // Left-top small
      display.fillRect(x + smallOffset2, y + smallOffset3, 1, 1, SSD1306_WHITE); // Left-bottom small
      display.fillRect(x + size - smallOffset2, y + smallOffset1, 1, 1, SSD1306_WHITE); // Right-top small
      display.fillRect(x + size - smallOffset2, y + smallOffset3, 1, 1, SSD1306_WHITE); // Right-bottom small
      display.fillRect(x + smallOffset1, y + size - smallOffset2, 1, 1, SSD1306_WHITE); // Bottom-left small
      display.fillRect(x + smallOffset3, y + size - smallOffset2, 1, 1, SSD1306_WHITE); // Bottom-right small
    } else {
      // Night: Clear sky with moon and stars - Scaled
      int moonOffsetX = SCALE_COORD(3);
      int moonOffsetY = SCALE_COORD(2);
      int moonRadius = SCALE_COORD(8);
      // Crescent moon (right side)
      display.fillCircle(centerX + moonOffsetX, centerY - moonOffsetY, moonRadius, SSD1306_WHITE);
      display.fillCircle(centerX, centerY - moonOffsetY, moonRadius, SSD1306_BLACK);
      
      // Stars (small 4-pointed stars scattered) - scaled positions
      int star1X = SCALE_COORD(6), star1Y = SCALE_COORD(6);
      int star2X = SCALE_COORD(28), star2Y = SCALE_COORD(8);
      int star3X = SCALE_COORD(5), star3Y = SCALE_COORD(28);
      int star4X = SCALE_COORD(30), star4Y = SCALE_COORD(30);
      
      // Top-left star
      display.fillRect(x + star1X, y + star1Y, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star1X - 1, y + star1Y + 1, 3, 1, SSD1306_WHITE);
      display.fillRect(x + star1X, y + star1Y + 2, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star1X + 1, y + star1Y - 1, 1, 3, SSD1306_WHITE);
      
      // Top-right star
      display.fillRect(x + star2X, y + star2Y, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star2X - 1, y + star2Y + 1, 3, 1, SSD1306_WHITE);
      display.fillRect(x + star2X, y + star2Y + 2, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star2X + 1, y + star2Y - 1, 1, 3, SSD1306_WHITE);
      
      // Bottom-left star
      display.fillRect(x + star3X, y + star3Y, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star3X - 1, y + star3Y + 1, 3, 1, SSD1306_WHITE);
      display.fillRect(x + star3X, y + star3Y + 2, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star3X + 1, y + star3Y - 1, 1, 3, SSD1306_WHITE);
      
      // Bottom-right star
      display.fillRect(x + star4X, y + star4Y, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star4X - 1, y + star4Y + 1, 3, 1, SSD1306_WHITE);
      display.fillRect(x + star4X, y + star4Y + 2, 1, 1, SSD1306_WHITE);
      display.fillRect(x + star4X + 1, y + star4Y - 1, 1, 3, SSD1306_WHITE);
    }
  } else if (icon == 1) {
    // Partly Cloudy/Sunny - sun peeking behind small cloud - Scaled
    // Sun (top-left area)
    display.fillCircle(x + SCALE_COORD(9), y + SCALE_COORD(9), SCALE_COORD(7), SSD1306_WHITE);
    display.fillRect(x + SCALE_COORD(9), y + SCALE_COORD(2), SCALE_COORD(2), SCALE_COORD(4), SSD1306_WHITE); // Top ray
    display.fillRect(x + SCALE_COORD(2), y + SCALE_COORD(9), SCALE_COORD(4), SCALE_COORD(2), SSD1306_WHITE); // Left ray
    // Cloud (bottom-right, overlapping sun slightly)
    display.fillCircle(x + SCALE_COORD(20), y + SCALE_COORD(20), SCALE_COORD(6), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(27), y + SCALE_COORD(21), SCALE_COORD(5), SSD1306_WHITE);
    display.fillRect(x + SCALE_COORD(19), y + SCALE_COORD(21), SCALE_COORD(9), SCALE_COORD(5), SSD1306_WHITE);
  } else if (icon == 2) {
    // Cloudy - full cloud cover - Scaled
    display.fillCircle(x + SCALE_COORD(9), y + SCALE_COORD(8), SCALE_COORD(7), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(18), y + SCALE_COORD(8), SCALE_COORD(8), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(27), y + SCALE_COORD(9), SCALE_COORD(6), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(13), y + SCALE_COORD(11), SCALE_COORD(5), SSD1306_WHITE);
    display.fillRect(x + SCALE_COORD(8), y + SCALE_COORD(10), SCALE_COORD(22), SCALE_COORD(8), SSD1306_WHITE);
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
    // Overcast/Cloudy - full cloud cover (no sun visible) - Scaled
    display.fillCircle(x + SCALE_COORD(9), y + SCALE_COORD(8), SCALE_COORD(7), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(18), y + SCALE_COORD(8), SCALE_COORD(8), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(27), y + SCALE_COORD(9), SCALE_COORD(6), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(13), y + SCALE_COORD(11), SCALE_COORD(5), SSD1306_WHITE);
    display.fillRect(x + SCALE_COORD(8), y + SCALE_COORD(10), SCALE_COORD(22), SCALE_COORD(8), SSD1306_WHITE);
  } else {
    // Default - generic cloud for invalid icon codes (outside 0-9) - Scaled
    display.fillCircle(x + SCALE_COORD(9), y + SCALE_COORD(8), SCALE_COORD(7), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(18), y + SCALE_COORD(8), SCALE_COORD(8), SSD1306_WHITE);
    display.fillCircle(x + SCALE_COORD(27), y + SCALE_COORD(9), SCALE_COORD(6), SSD1306_WHITE);
    display.fillRect(x + SCALE_COORD(8), y + SCALE_COORD(10), SCALE_COORD(21), SCALE_COORD(7), SSD1306_WHITE);
  }
  
  #undef SCALE_COORD
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
  
  // Layout calculations:
  // Screen: 128x64 pixels
  // Header: 10px tall (Y 0-9)
  // Gap: 2px (Y 10-11)
  // Content area: 43px tall (Y 12-54)
  // Bottom bar: 9px tall (Y 55-63)
  // Total: 10 + 2 + 43 + 9 = 64px
    
  // 1. Location - Top header bar (Y: 0-9)
  int headerHeight = 10;
  display.fillRect(0, 0, SCREEN_WIDTH, headerHeight, SSD1306_WHITE);
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
  display.setCursor(textX, 1);
  display.print(displayText);
  
  display.setTextColor(SSD1306_WHITE);
  
  // Main content area (Y: 12-54, 43px height available, bottom bar starts at Y:55)
  // Split: 40% left (51px) for large icon, 60% right (77px) for temp, H, L
  
  // LEFT 40% (0-50px, 51px wide): Large icon with proper spacing and perfect centering
  int leftAreaWidth = 51; // 40% of 128 (pixels 0-50, width 51)
  int headerGap = 2; // Gap between header and content
  int contentTop = headerHeight + headerGap; // 10 + 2 = 12
  int bottomBarHeight = 9;
  int contentBottom = SCREEN_HEIGHT - bottomBarHeight - 1; // 64 - 9 - 1 = 54
  int availableHeight = contentBottom - contentTop + 1; // 54 - 12 + 1 = 43px total
  
  // Add 1px spacing on top and bottom as requested
  int topSpacing = 1;
  int bottomSpacing = 1;
  int iconSize = availableHeight - topSpacing - bottomSpacing; // 43 - 1 - 1 = 41px
  
  // Perfect horizontal centering in left area
  // Left area center: (leftAreaWidth - 1) / 2 = 50 / 2 = 25 (pixel 25 is center of 0-50)
  // Icon center should align with area center: iconX + iconSize/2 = 25
  // Therefore: iconX = 25 - iconSize/2
  int leftAreaCenter = (leftAreaWidth - 1) / 2; // 25 (center pixel of 0-50 range)
  int iconX = leftAreaCenter - (iconSize / 2); // 25 - 20 = 5 (for 41px icon, center at 25)
  
  // Vertical positioning with 1px spacing from top and bottom
  int iconY = contentTop + topSpacing; // 12 + 1 = 13 (1px below content top)
  // Icon ends at: iconY + iconSize = 13 + 41 = 54 (1px above bottom bar at Y:55)
  
  drawWeatherIcon(weather.icon, iconX, iconY, iconSize);
  
  // RIGHT 60% (52-128px): Current temp (centered), H, L
  int rightStartX = 52; // Start of right area (40% of 128)
  int rightAreaWidth = 76; // 60% of 128 (128 - 52)
  int rightCenterX = rightStartX + (rightAreaWidth / 2); // Center of right area
  
  // Current temperature - Centered in right area, at top
  // Text size 2 is ~12px per digit, size 1 is ~6px per char
  // Position: Y 15 (3px below content top at Y 12, accounting for text size 2 height ~12px)
  display.setTextSize(2);
  int tempDigits = weather.temp < 10 ? 1 : 2;
  int tempWidth = tempDigits * 12 + 6; // Temp digits (size 2) + "C" (size 1)
  int tempX = rightCenterX - (tempWidth / 2);
  int tempY = contentTop + 3; // 12 + 3 = 15
  display.setCursor(tempX, tempY);
  display.setTextSize(2);
  display.print(weather.temp);
  display.setTextSize(1);
  display.print("C");
  
  // UV and Pressure - Below temp, above H/L, centered in right area
  // Position: Y 34 (19px below temp at Y 15, with 8px line spacing)
  display.setTextSize(1);
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
  int uvPY = tempY + 19; // 15 + 19 = 34 (below temp with spacing)
  
  display.setCursor(uvX, uvPY);
  display.print("UV:");
  display.print(uvValue);
  
  display.setCursor(pX, uvPY);
  display.print("P:");
  display.print(pressureValue);
  
  // High/Low temperatures - Below UV/P, centered in right area
  // Position: Y 45 (11px below UV/P at Y 34, with 8px line spacing + 3px gap)
  // Calculate widths directly (H: + 1-2 digits + C = 4-5 chars, same for L:)
  int highDigits = weather.high < 10 ? 1 : 2;
  int lowDigits = weather.low < 10 ? 1 : 2;
  int highWidth = (2 + highDigits + 1) * 6; // "H:" + digits + "C"
  int lowWidth = (2 + lowDigits + 1) * 6; // "L:" + digits + "C"
  int spacing = 10;
  int totalWidth = highWidth + spacing + lowWidth;
  int highX = rightCenterX - (totalWidth / 2);
  int lowX = highX + highWidth + spacing;
  int hlY = uvPY + 11; // 34 + 11 = 45 (below UV/P with spacing)
  
  display.setCursor(highX, hlY);
  display.setTextSize(1);
  display.print("H:");
  display.print(weather.high);
  display.print("C");
  
  display.setCursor(lowX, hlY);
  display.setTextSize(1);
  display.print("L:");
  display.print(weather.low);
  display.print("C");
  
  // 6. Time (DD/MM hh:mm) - Bottom bar (Y: 55-63)
  int bottomBarY = contentBottom + 1; // 54 + 1 = 55
  display.fillRect(0, bottomBarY, SCREEN_WIDTH, bottomBarHeight, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  // Center the time text - format: DD/MM hh:mm = 11 chars = 66px (fixed)
  int day = rtc.getDay();
  int month = rtc.getMonth() + 1; // ESP32Time returns 0-11, convert to 1-12 for display
  int hour = rtc.getHour();
  int minute = rtc.getMinute();
  int timeTextWidth = 66; // Fixed width: 11 chars * 6px = 66px
  int timeX = (SCREEN_WIDTH - timeTextWidth) / 2;
  int timeY = bottomBarY + 1; // 55 + 1 = 56 (1px below bottom bar top)
  display.setCursor(timeX, timeY);
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

