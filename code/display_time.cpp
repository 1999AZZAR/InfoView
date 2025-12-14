/*
 * Display Time - Time display functions
 */

#include "display_time.h"
#include "config.h"
#include <string.h>

// External objects
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;

void displayTime() {
  // Modern watch face design
  
  // Top decorative line - Phone battery indicator (full line = full battery)
  // Cache connection state (checked in display_manager, but we need it here too)
  static bool cachedConnected = false;
  static unsigned long lastConnCheck = 0;
  unsigned long now = millis();
  // Update connection cache every 500ms
  if (now - lastConnCheck >= 500) {
    cachedConnected = chronos.isConnected();
    lastConnCheck = now;
  }
  bool isConnected = cachedConnected;
  
  int batteryLevel = 0;
  if (isConnected) {
    // Get phone battery level from ChronosESP32
    batteryLevel = chronos.getPhoneBattery();
    // Ensure battery level is in valid range (0-100)
    if (batteryLevel < 0) batteryLevel = 0;
    if (batteryLevel > 100) batteryLevel = 100;
  }
  
  // Draw battery indicator line (top line, proportional to battery level)
  // Full line (128px) = 100% battery
  if (isConnected && batteryLevel > 0) {
    int batteryWidth = (batteryLevel * SCREEN_WIDTH) / 100;
    if (batteryWidth > 0) {
      display.drawLine(0, 0, batteryWidth - 1, 0, SSD1306_WHITE);
    }
  }
  
  // Large time display - hh:mm:ss all in one line, centered
  int hour = chronos.getHourC();
  int minute = rtc.getMinute();
  int second = rtc.getSecond();
  
  // Calculate width directly: 8 chars (hh:mm:ss) = 96px at size 2
  display.setTextSize(2);
  int timeX = (SCREEN_WIDTH - 96) / 2; // Fixed width for hh:mm:ss
  display.setCursor(timeX, 18);
  // Print directly without String concatenation
  if (hour < 10) display.print("0");
  display.print(hour);
  display.print(":");
  if (minute < 10) display.print("0");
  display.print(minute);
  display.print(":");
  if (second < 10) display.print("0");
  display.print(second);
  
  // Date display - centered below time
  int day = rtc.getDay();
  int month = rtc.getMonth() + 1; // ESP32Time returns 0-11, convert to 1-12 for display
  int year = rtc.getYear();
  
  // Day name - full day name (no padding)
  const char* dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  int dayOfWeek = rtc.getDayofWeek();
  
  // Calculate date width: dayName (varies) + space + DD/MM/YYYY (10 chars) = ~6-13 chars for dayName + 10
  // Longest day name is "Wednesday" = 9 chars, so max = 9 + 1 + 10 = 20 chars = 120px
  // We'll calculate dynamically but use direct printing
  display.setTextSize(1);
  const char* dayName = dayNames[dayOfWeek];
  int dayNameLen = strlen(dayName);
  int dateWidth = (dayNameLen + 1 + 10) * 6; // dayName + space + DD/MM/YYYY
  int dateX = (SCREEN_WIDTH - dateWidth) / 2;
  display.setCursor(dateX, 38);
  // Print directly without String concatenation
  display.print(dayName);
  display.print(" ");
  if (day < 10) display.print("0");
  display.print(day);
  display.print("/");
  if (month < 10) display.print("0");
  display.print(month);
  display.print("/");
  display.print(year);
  
  // Bottom decorative line - Only show when connected
  if (isConnected) {
    display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  }
}

