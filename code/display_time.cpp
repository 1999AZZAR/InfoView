/*
 * Display Time - Time display functions
 */

#include "display_time.h"
#include "config.h"

// External objects
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;
extern ESP32Time rtc;

void displayTime() {
  // Modern watch face design
  
  // Top decorative line
  display.drawLine(0, 0, 127, 0, SSD1306_WHITE);
  
  // Large time display - hh:mm:ss all in one line, centered
  int hour = chronos.getHourC();
  int minute = rtc.getMinute();
  int second = rtc.getSecond();
  
  // Build complete time string hh:mm:ss
  String timeStr = "";
  if (hour < 10) timeStr += "0";
  timeStr += String(hour);
  timeStr += ":";
  if (minute < 10) timeStr += "0";
  timeStr += String(minute);
  timeStr += ":";
  if (second < 10) timeStr += "0";
  timeStr += String(second);
  
  // Center the time (size 2 to fit hh:mm:ss on one line)
  display.setTextSize(2);
  int timeWidth = timeStr.length() * 12; // Approximate width for size 2 (8 chars = hh:mm:ss = ~96px)
  int timeX = (SCREEN_WIDTH - timeWidth) / 2;
  display.setCursor(timeX, 18);
  display.print(timeStr);
  
  // Date display - centered below time
  int day = rtc.getDay();
  int month = rtc.getMonth();
  int year = rtc.getYear();
  
  // Day name - full day name (no padding)
  const char* dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  int dayOfWeek = rtc.getDayofWeek();
  
  // Build date string
  String dateStr = String(dayNames[dayOfWeek]);
  dateStr += " ";
  if (day < 10) dateStr += "0";
  dateStr += String(day);
  dateStr += "/";
  if (month < 10) dateStr += "0";
  dateStr += String(month);
  dateStr += "/";
  dateStr += String(year);
  
  // Center the date
  display.setTextSize(1);
  int dateWidth = dateStr.length() * 6; // Approximate width for size 1
  int dateX = (SCREEN_WIDTH - dateWidth) / 2;
  display.setCursor(dateX, 38);
  display.print(dateStr);
  
  // Bottom decorative line
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
}

