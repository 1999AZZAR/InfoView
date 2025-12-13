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

  // Top line: App name (left) + Notification count (right)
  display.setTextSize(1);
  
  // App name on the left - use directly, no String copy
  if (currentNotification.app.length() == 0) {
    display.setCursor(2, 2);
    display.print("Unknown App");
  } else {
    display.setCursor(2, 2);
    display.print(currentNotification.app);
  }
  
  // Notification count on the right (e.g., "1/4") - calculate width dynamically
  // Count digits for proper alignment
  int currentNum = currentNotificationIndex + 1;
  int totalNum = notificationQueueCount;
  int currentDigits = (currentNum < 10) ? 1 : 2;
  int totalDigits = (totalNum < 10) ? 1 : 2;
  int numberWidth = (currentDigits + 1 + totalDigits) * 6; // digits + "/" + digits
  int numberX = SCREEN_WIDTH - numberWidth - 2; // Right aligned with 2px margin
  display.setCursor(numberX, 2);
  display.print(currentNum);
  display.print("/");
  display.print(totalNum);
  
  // Top separator line
  display.drawLine(0, 11, SCREEN_WIDTH - 1, 11, SSD1306_WHITE);
  
  // Content (message) as 5 lines with word wrapping and ellipsis
  String content = currentNotification.message;
  if (content.length() == 0) {
    content = currentNotification.title; // Fallback to title if no message
  }
  if (content.length() == 0) {
    content = "No content";
  }
  
  // Clean up content: trim and collapse multiple spaces
  content.trim();
  // Collapse multiple spaces into single space
  while (content.indexOf("  ") >= 0) {
    content.replace("  ", " ");
  }
  
  // Content area: Y: 14-64 (50px for 5 lines at 10px per line)
  int contentStartY = 14;
  int lineHeight = 10;
  int maxLines = 5;
  int contentAreaWidth = SCREEN_WIDTH - 4; // Leave 2px margin on each side
  int maxCharsPerLine = contentAreaWidth / 6; // ~20 chars per line
  
  // Word wrap and display content across 5 lines
  int currentPos = 0;
  int linesDisplayed = 0;
  int currentY = contentStartY;
  
  // Skip leading whitespace
  while (currentPos < (int)content.length() && content.charAt(currentPos) == ' ') {
    currentPos++;
  }
  
  while (currentPos < (int)content.length() && linesDisplayed < maxLines) {
    // Skip any leading spaces on new line
    while (currentPos < (int)content.length() && content.charAt(currentPos) == ' ') {
      currentPos++;
    }
    if (currentPos >= (int)content.length()) break;
    
    bool isLastLine = (linesDisplayed == maxLines - 1);
    
    // Calculate how many characters fit on this line
    int charsToShow = min(maxCharsPerLine, (int)content.length() - currentPos);
    String line = content.substring(currentPos, currentPos + charsToShow);
    
    // If not at the end and not the last line, try to break at word boundary
    bool hasMoreContent = (currentPos + charsToShow < (int)content.length());
    if (!isLastLine && hasMoreContent) {
      // Find last space in the original substring (before trimming)
      int lastSpace = line.lastIndexOf(' ');
      if (lastSpace > 0 && lastSpace > maxCharsPerLine / 2) {
        // Break at word boundary
        line = line.substring(0, lastSpace);
        charsToShow = lastSpace + 1;
        // Skip the space and any following spaces in original content
        while (currentPos + charsToShow < (int)content.length() && 
               content.charAt(currentPos + charsToShow) == ' ') {
          charsToShow++;
        }
      }
    }
    
    // Trim trailing spaces from line (after word break decision)
    line.trim();
    
    // Check if this is the last line and we need ellipsis
    hasMoreContent = (currentPos + charsToShow < (int)content.length());
    
    if (isLastLine && hasMoreContent) {
      // Truncate and add ellipsis if needed
      int ellipsisChars = 3; // "..."
      int availableChars = maxCharsPerLine - ellipsisChars;
      if (line.length() > availableChars) {
        // Truncate at word boundary if possible
        int truncatePos = availableChars;
        int lastSpace = line.lastIndexOf(' ');
        if (lastSpace > availableChars / 2) {
          truncatePos = lastSpace;
        }
        line = line.substring(0, truncatePos);
        line.trim(); // Remove trailing spaces
        charsToShow = truncatePos;
      }
      line += "...";
    }
    
    // Only display non-empty lines
    if (line.length() > 0) {
      display.setCursor(2, currentY);
      display.print(line);
      currentY += lineHeight;
      linesDisplayed++;
    }
    
    // Update position: move past what we displayed
    currentPos += charsToShow;
    
    // If we added ellipsis, stop processing
    if (isLastLine && hasMoreContent) {
      break;
    }
  }
}

