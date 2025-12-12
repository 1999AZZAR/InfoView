/*
 * Display Notification - Notification display functions
 */

#include "display_notification.h"
#include "notification_queue.h"
#include "config.h"

// External objects
extern Adafruit_SSD1306 display;
extern Notification notificationQueue[];
extern int notificationQueueCount;
extern int currentNotificationIndex;

void displayNotification() {
  if (notificationQueueCount == 0) {
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println("No notification");
    return;
  }

  // Get current notification from queue
  Notification currentNotification = notificationQueue[currentNotificationIndex];

  // 1. Notification number at top (e.g., "1/4")
  display.setTextSize(1);
  String numberStr = String(currentNotificationIndex + 1) + "/" + String(notificationQueueCount);
  int numberWidth = numberStr.length() * 6;
  int numberX = (SCREEN_WIDTH - numberWidth) / 2;
  display.setCursor(numberX, 2);
  display.print(numberStr);
  
  // Top separator line
  display.drawLine(0, 11, SCREEN_WIDTH - 1, 11, SSD1306_WHITE);
  
  // 2. App name (centered, below number)
  String appName = currentNotification.app;
  if (appName.length() == 0) {
    appName = "Unknown App";
  }
  int appWidth = appName.length() * 6;
  int appX = (SCREEN_WIDTH - appWidth) / 2;
  display.setCursor(appX, 14);
  display.setTextSize(1);
  display.print(appName);
  
  // Middle separator line
  display.drawLine(0, 24, SCREEN_WIDTH - 1, 24, SSD1306_WHITE);
  
  // 3. Content (message) as 3 lines with word wrapping and ellipsis
  String content = currentNotification.message;
  if (content.length() == 0) {
    content = currentNotification.title; // Fallback to title if no message
  }
  if (content.length() == 0) {
    content = "No content";
  }
  
  // Content area: Y: 28-58 (30px for 3 lines at 10px per line)
  int contentStartY = 28;
  int lineHeight = 10;
  int maxLines = 3;
  int contentAreaWidth = SCREEN_WIDTH - 4; // Leave 2px margin on each side
  int maxCharsPerLine = contentAreaWidth / 6; // ~20 chars per line
  
  // Word wrap and display content across 3 lines
  int currentPos = 0;
  int linesDisplayed = 0;
  int currentY = contentStartY;
  
  while (currentPos < (int)content.length() && linesDisplayed < maxLines) {
    // Calculate how many characters fit on this line
    int charsToShow = min(maxCharsPerLine, (int)content.length() - currentPos);
    String line = content.substring(currentPos, currentPos + charsToShow);
    
    // If not at the end and not the last line, try to break at word boundary
    if (currentPos + charsToShow < (int)content.length() && linesDisplayed < maxLines - 1) {
      int lastSpace = line.lastIndexOf(' ');
      if (lastSpace > 0 && lastSpace > maxCharsPerLine / 2) {
        line = line.substring(0, lastSpace);
        charsToShow = lastSpace + 1;
      }
    }
    
    // Check if this is the last line and we need ellipsis
    bool isLastLine = (linesDisplayed == maxLines - 1);
    bool hasMoreContent = (currentPos + charsToShow < (int)content.length());
    
    if (isLastLine && hasMoreContent) {
      // Truncate and add ellipsis if needed
      int ellipsisChars = 3; // "..."
      int availableChars = maxCharsPerLine - ellipsisChars;
      if (line.length() > availableChars) {
        line = line.substring(0, availableChars);
        charsToShow = availableChars;
      }
      line += "...";
    }
    
    display.setCursor(2, currentY);
    display.print(line);
    
    currentY += lineHeight;
    linesDisplayed++;
    currentPos += charsToShow;
    
    // If we added ellipsis, stop processing
    if (isLastLine && hasMoreContent) {
      break;
    }
  }
  
  // Bottom separator line
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
}

