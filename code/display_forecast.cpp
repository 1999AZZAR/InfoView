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
  display.fillRect(0, 0, SCREEN_WIDTH, 9, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  const char* title = "Forecast";
  int titleWidth = strlen(title) * 6;
  int titleX = (SCREEN_WIDTH - titleWidth) / 2;
  display.setCursor(titleX, 1);
  display.print(title);
  display.setTextColor(SSD1306_WHITE);
  
  // Display up to 4 forecast entries (skip index 0 which is current weather)
  // Layout: 2 rows x 2 columns of forecast items
  // Each item: Small icon (20x20) + temp + H/L
  // 
  // Layout calculations:
  // Screen: 128x64 pixels
  // Header: 9px tall (Y 0-8)
  // Available height: 55px (Y 9-63)
  // Item dimensions: 64px wide x 27px tall each
  //   - Top row: Y 9-35 (27px)
  //   - Bottom row: Y 36-62 (27px)
  // Each item contains:
  //   - Icon: 20x20px at (startX+2, startY+2), but some icons draw beyond bounds
  //   - Icon area reserved: 38px from left edge (covers worst case overflow to x+35)
  //   - Text area: starts at X=startX+38, ~26px wide (sufficient for all text)
  //   - Text lines: Temp at Y=textY, H: at Y=textY+8, L: at Y=textY+16
  int maxForecastItems = min(4, forecastCount - 1); // Skip current (index 0)
  int itemWidth = SCREEN_WIDTH / 2; // 64px per item
  int headerHeight = 9; // Header occupies Y 0-8
  int availableHeight = SCREEN_HEIGHT - headerHeight; // 55px available (Y 9-63)
  int itemHeight = availableHeight / 2; // 27px per row (27.5 rounded down)
  int iconSize = 20; // Small icons for forecast
  
  for (int i = 0; i < maxForecastItems; i++) {
    int forecastIndex = i + 1; // Skip index 0 (current weather)
    Weather forecast = chronos.getWeatherAt(forecastIndex);
    
    // Calculate position: row and column
    int row = i / 2;
    int col = i % 2;
    int startX = col * itemWidth;
    // Top row starts at headerHeight (Y=9), bottom row starts at headerHeight + itemHeight (Y=36)
    int startY = headerHeight + (row * itemHeight);
    
    // Draw divider lines between items
    if (col == 1) {
      // Vertical divider between columns (at X=64, from header to bottom)
      display.drawLine(startX, headerHeight, startX, SCREEN_HEIGHT - 1, SSD1306_WHITE);
    }
    if (row == 1) {
      // Horizontal divider between rows (at Y=36, full width)
      display.drawLine(0, startY, SCREEN_WIDTH - 1, startY, SSD1306_WHITE);
    }
    
    // Draw small icon (20x20) at top-left of item with padding
    // Icon area: left side of item
    int padding = 2; // 2px padding from edges
    int iconX = startX + padding;
    int iconY = startY + padding;
    
    // Use the existing drawWeatherIcon function with 20x20 size
    // WARNING: Some icons (3-8) use hardcoded coordinates for 36px icons
    // These will draw beyond the 20px bounds (up to x+26, x+33, etc.)
    // We must reserve enough space to prevent text overlap
    drawWeatherIcon(forecast.icon, iconX, iconY, iconSize);
    
    // Temperature and text on the right side of item
    // CRITICAL: Some icons (3-8) use hardcoded coordinates for 36px icons
    // Worst case: fog icon (7) draws lines to x+33 from icon start
    // Icon starts at startX + 2, worst case draws to startX + 2 + 33 = startX + 35
    // Reserve 38px from left edge (covers x+35 + 3px safety margin)
    int iconAreaReserved = 38; // Reserve 38px for icon area (safe margin after worst case)
    int textStartX = startX + iconAreaReserved; // Start text at startX + 38
    
    // Verify text fits within item bounds (each item is 64px wide)
    // Available text width: itemWidth - iconAreaReserved = 64 - 38 = 26px
    // This is enough for: "XXC" (~18px), "H:XX" (~24px), "L:XX" (~24px)
    // For left column: textStartX = 0 + 38 = 38, available = 26px
    // For right column: textStartX = 64 + 38 = 102, available = 26px
    
    // Text Y position: align with icon top
    int textY = startY + padding; // Same Y as icon top (startY + 2)
    
    // Temperature (top)
    display.setCursor(textStartX, textY);
    display.setTextSize(1);
    display.print(forecast.temp);
    display.print("C");
    
    // High/Low below temp (compact, 8px line spacing)
    int lineHeight = 8; // Standard line height for size 1 text
    display.setCursor(textStartX, textY + lineHeight);
    display.print("H:");
    display.print(forecast.high);
    display.setCursor(textStartX, textY + (lineHeight * 2));
    display.print("L:");
    display.print(forecast.low);
  }
}

