/*
 * Display Forecast - Weather forecast display functions
 */

#include "display_forecast.h"
#include "display_weather.h"
#include "config.h"
#include <string.h>

// External objects
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;

void displayForecast() {
  int forecastCount = chronos.getWeatherCount();
  
  // Need at least 2 entries for forecast (current + at least 1 future)
  if (forecastCount < 2) {
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println("No forecast");
    display.println("data available");
    display.setCursor(0, 40);
    display.println("Check Chronos");
    display.println("app settings");
    return;
  }
  
  // Top header: "Forecast" title
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  const char* title = "Forecast";
  int titleWidth = strlen(title) * 6;
  int titleX = (SCREEN_WIDTH - titleWidth) / 2;
  display.setCursor(titleX, 2);
  display.print(title);
  display.setTextColor(SSD1306_WHITE);
  
  // Display up to 4 forecast entries (skip index 0 which is current weather)
  // Layout: 2 rows x 2 columns of forecast items
  // Each item: Small icon (20x20) + temp + H/L
  int maxForecastItems = min(4, forecastCount - 1); // Skip current (index 0)
  int itemWidth = SCREEN_WIDTH / 2; // 64px per item
  int itemHeight = (SCREEN_HEIGHT - 11) / 2; // ~26px per item (53px / 2) - full height available
  int iconSize = 20; // Small icons for forecast
  
  for (int i = 0; i < maxForecastItems; i++) {
    int forecastIndex = i + 1; // Skip index 0 (current weather)
    Weather forecast = chronos.getWeatherAt(forecastIndex);
    
    // Calculate position: row and column
    int row = i / 2;
    int col = i % 2;
    int startX = col * itemWidth;
    // Top row (F1, F2) starts at 12 (1px below header for more room)
    // Bottom row (F3, F4) starts 1px lower to give top row more space
    int startY = 12 + (row * itemHeight) + (row > 0 ? 1 : 0);
    
    // Draw divider lines between items (adjust for new positioning)
    if (col == 1) {
      // Vertical divider between columns
      int dividerStartY = 12; // Start from top row
      int dividerEndY = SCREEN_HEIGHT; // Go to bottom
      display.drawLine(startX, dividerStartY, startX, dividerEndY, SSD1306_WHITE);
    }
    if (row == 1) {
      // Horizontal divider between rows (at the adjusted position)
      display.drawLine(startX, startY, startX + itemWidth, startY, SSD1306_WHITE);
    }
    
    // Draw small icon (20x20) at top-left of item
    int iconX = startX + 2;
    int iconY = startY + 2;
    
    // Draw scaled-down icon (simplified version)
    display.setTextSize(1);
    int centerX = iconX + iconSize / 2;
    int centerY = iconY + iconSize / 2;
    
    // Use the existing drawWeatherIcon function but scale it down
    // For forecast, we'll use a simplified approach - reuse icon drawing logic
    // Draw icon at smaller scale (20x20 instead of 36x36)
    int hour = chronos.getHourC();
    bool isDay = (hour >= 6 && hour < 18);
    
    // Simplified icon drawing for forecast (20x20 size)
    if (forecast.icon == 0) {
      // Clear - sun or moon
      if (isDay) {
        display.fillCircle(centerX, centerY, 5, SSD1306_WHITE);
        display.drawCircle(centerX, centerY, 3, SSD1306_BLACK);
      } else {
        display.fillCircle(centerX + 1, centerY - 1, 4, SSD1306_WHITE);
        display.fillCircle(centerX, centerY - 1, 4, SSD1306_BLACK);
        // Small stars
        display.drawPixel(iconX + 2, iconY + 2, SSD1306_WHITE);
        display.drawPixel(iconX + 16, iconY + 4, SSD1306_WHITE);
      }
    } else if (forecast.icon == 1) {
      // Partly cloudy
      display.fillCircle(iconX + 5, iconY + 5, 4, SSD1306_WHITE);
      display.fillCircle(iconX + 12, iconY + 9, 3, SSD1306_WHITE);
      display.fillRect(iconX + 11, iconY + 9, 5, 2, SSD1306_WHITE);
    } else if (forecast.icon == 2 || forecast.icon == 9) {
      // Cloudy
      display.fillCircle(iconX + 5, iconY + 7, 4, SSD1306_WHITE);
      display.fillCircle(iconX + 11, iconY + 7, 4, SSD1306_WHITE);
      display.fillRect(iconX + 4, iconY + 8, 9, 3, SSD1306_WHITE);
    } else if (forecast.icon == 3 || forecast.icon == 4) {
      // Rain
      display.fillCircle(iconX + 5, iconY + 6, 3, SSD1306_WHITE);
      display.fillCircle(iconX + 11, iconY + 6, 3, SSD1306_WHITE);
      display.fillRect(iconX + 4, iconY + 7, 8, 2, SSD1306_WHITE);
      display.fillRect(iconX + 6, iconY + 11, 1, 4, SSD1306_WHITE);
      display.fillRect(iconX + 9, iconY + 12, 1, 4, SSD1306_WHITE);
    } else if (forecast.icon == 5) {
      // Storm
      display.fillCircle(iconX + 5, iconY + 6, 3, SSD1306_WHITE);
      display.fillCircle(iconX + 11, iconY + 6, 3, SSD1306_WHITE);
      display.fillRect(iconX + 4, iconY + 7, 8, 2, SSD1306_WHITE);
      display.fillRect(iconX + 7, iconY + 10, 2, 5, SSD1306_WHITE);
    } else if (forecast.icon == 6) {
      // Snow
      display.fillCircle(iconX + 5, iconY + 6, 3, SSD1306_WHITE);
      display.fillCircle(iconX + 11, iconY + 6, 3, SSD1306_WHITE);
      display.fillRect(iconX + 4, iconY + 7, 8, 2, SSD1306_WHITE);
      // Small snowflake
      display.fillRect(iconX + 9, iconY + 11, 1, 3, SSD1306_WHITE);
      display.fillRect(iconX + 8, iconY + 12, 3, 1, SSD1306_WHITE);
    } else if (forecast.icon == 7) {
      // Fog
      display.drawLine(iconX + 3, iconY + 7, iconX + 15, iconY + 8, SSD1306_WHITE);
      display.drawLine(iconX + 3, iconY + 10, iconX + 15, iconY + 11, SSD1306_WHITE);
      display.drawLine(iconX + 3, iconY + 13, iconX + 15, iconY + 14, SSD1306_WHITE);
    } else if (forecast.icon == 8) {
      // Drizzle
      display.fillCircle(iconX + 5, iconY + 7, 3, SSD1306_WHITE);
      display.fillCircle(iconX + 11, iconY + 7, 3, SSD1306_WHITE);
      display.fillRect(iconX + 4, iconY + 8, 8, 2, SSD1306_WHITE);
      display.drawPixel(iconX + 6, iconY + 12, SSD1306_WHITE);
      display.drawPixel(iconX + 9, iconY + 13, SSD1306_WHITE);
    } else {
      // Default cloud
      display.fillCircle(iconX + 5, iconY + 7, 4, SSD1306_WHITE);
      display.fillCircle(iconX + 11, iconY + 7, 4, SSD1306_WHITE);
      display.fillRect(iconX + 4, iconY + 8, 9, 3, SSD1306_WHITE);
    }
    
    // Temperature on the right side of icon area
    int tempX = iconX + iconSize + 2;
    int tempY = startY + 3;
    display.setCursor(tempX, tempY);
    display.setTextSize(1);
    display.print(forecast.temp);
    display.print("C");
    
    // High/Low below temp (compact)
    display.setCursor(tempX, tempY + 9);
    display.print("H:");
    display.print(forecast.high);
    display.setCursor(tempX, tempY + 17);
    display.print("L:");
    display.print(forecast.low);
  }
}

