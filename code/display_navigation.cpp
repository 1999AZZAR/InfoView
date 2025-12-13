/*
 * Display Navigation - Navigation display functions
 */

#include "display_navigation.h"
#include "config.h"

// External objects
extern Adafruit_SSD1306 display;
extern ChronosESP32 chronos;

void drawNavigationArrow(String direction, int x, int y) {
  direction.toLowerCase();
  int size = 40; // Larger arrow for better visibility
  int centerX = x + size/2;
  int centerY = y + size/2;
  
  // Roundabout (check first as it's most specific)
  if (direction.indexOf("roundabout") >= 0 || direction.indexOf("round about") >= 0 || direction.indexOf("traffic circle") >= 0) {
    // Roundabout icon - circle with arrow indicating exit direction
    int radius = size/3;
    
    // Draw roundabout circle
    display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
    display.drawCircle(centerX, centerY, radius - 1, SSD1306_WHITE);
    
    // Determine exit direction from text
    if (direction.indexOf("exit") >= 0) {
      // Try to find exit number or direction
      int exitPos = direction.indexOf("exit");
      String afterExit = direction.substring(exitPos + 4);
      afterExit.trim();
      
      // Check for left/right in exit direction
      if (afterExit.indexOf("left") >= 0 || afterExit.indexOf("1") >= 0 || afterExit.indexOf("2") >= 0) {
        // Exit left - arrow pointing left from circle
        display.fillTriangle(centerX - radius - 4, centerY, centerX - radius + 2, centerY - 6, centerX - radius + 2, centerY + 6, SSD1306_WHITE);
        display.fillRect(centerX - radius + 2, centerY - 3, 6, 6, SSD1306_WHITE);
      } else if (afterExit.indexOf("right") >= 0 || afterExit.indexOf("3") >= 0 || afterExit.indexOf("4") >= 0) {
        // Exit right - arrow pointing right from circle
        display.fillTriangle(centerX + radius + 4, centerY, centerX + radius - 2, centerY - 6, centerX + radius - 2, centerY + 6, SSD1306_WHITE);
        display.fillRect(centerX + radius - 8, centerY - 3, 6, 6, SSD1306_WHITE);
      } else {
        // Default exit straight/forward
        display.fillTriangle(centerX, centerY - radius - 4, centerX - 6, centerY - radius + 2, centerX + 6, centerY - radius + 2, SSD1306_WHITE);
        display.fillRect(centerX - 3, centerY - radius + 2, 6, 6, SSD1306_WHITE);
      }
    } else {
      // Default roundabout - arrow pointing up (straight through)
      display.fillTriangle(centerX, centerY - radius - 4, centerX - 6, centerY - radius + 2, centerX + 6, centerY - radius + 2, SSD1306_WHITE);
      display.fillRect(centerX - 3, centerY - radius + 2, 6, 6, SSD1306_WHITE);
    }
  }
  // Sharp turns (check before regular turns)
  else if (direction.indexOf("sharp left") >= 0 || direction.indexOf("hard left") >= 0) {
    // Sharp left arrow - more angled
    display.fillTriangle(x, centerY, centerX - 10, centerY - 14, centerX - 10, centerY - 6, SSD1306_WHITE);
    display.fillTriangle(x, centerY, centerX - 10, centerY + 14, centerX - 10, centerY + 6, SSD1306_WHITE);
    display.fillRect(centerX - 10, centerY - 7, size/2 + 2, 14, SSD1306_WHITE);
  } else if (direction.indexOf("sharp right") >= 0 || direction.indexOf("hard right") >= 0) {
    // Sharp right arrow - more angled
    display.fillTriangle(x + size, centerY, centerX + 10, centerY - 14, centerX + 10, centerY - 6, SSD1306_WHITE);
    display.fillTriangle(x + size, centerY, centerX + 10, centerY + 14, centerX + 10, centerY + 6, SSD1306_WHITE);
    display.fillRect(centerX, centerY - 7, size/2 + 2, 14, SSD1306_WHITE);
  }
  // Regular turns
  else if (direction.indexOf("left") >= 0 || direction.indexOf("turn left") >= 0) {
    // Complete left arrow (←)
    // Arrow head (pointing left)
    display.fillTriangle(x, centerY, centerX - 8, centerY - 12, centerX - 8, centerY - 4, SSD1306_WHITE);
    display.fillTriangle(x, centerY, centerX - 8, centerY + 12, centerX - 8, centerY + 4, SSD1306_WHITE);
    // Arrow shaft
    display.fillRect(centerX - 8, centerY - 6, size/2, 12, SSD1306_WHITE);
  } else if (direction.indexOf("right") >= 0 || direction.indexOf("turn right") >= 0) {
    // Complete right arrow (→)
    // Arrow head (pointing right)
    display.fillTriangle(x + size, centerY, centerX + 8, centerY - 12, centerX + 8, centerY - 4, SSD1306_WHITE);
    display.fillTriangle(x + size, centerY, centerX + 8, centerY + 12, centerX + 8, centerY + 4, SSD1306_WHITE);
    // Arrow shaft
    display.fillRect(centerX, centerY - 6, size/2, 12, SSD1306_WHITE);
  }
  // U-turn
  else if (direction.indexOf("u-turn") >= 0 || direction.indexOf("uturn") >= 0 || direction.indexOf("turn around") >= 0) {
    // Complete U-turn arrow
    // U shape
    display.drawCircle(centerX, centerY, size/3, SSD1306_WHITE);
    display.drawCircle(centerX, centerY, size/3 - 1, SSD1306_WHITE);
    // Arrow pointing up from U
    display.fillTriangle(centerX, y, centerX - 6, y + 8, centerX + 6, y + 8, SSD1306_WHITE);
  }
  // Slight turns
  else if (direction.indexOf("slight left") >= 0 || direction.indexOf("bear left") >= 0) {
    // Slight left arrow / Bear left
    // Curved arrow pointing slightly left
    display.fillTriangle(x + 5, centerY, centerX - 5, centerY - 8, centerX - 5, centerY - 2, SSD1306_WHITE);
    display.fillRect(centerX - 5, centerY - 4, size/2 - 5, 8, SSD1306_WHITE);
  } else if (direction.indexOf("slight right") >= 0 || direction.indexOf("bear right") >= 0) {
    // Slight right arrow / Bear right
    // Curved arrow pointing slightly right
    display.fillTriangle(x + size - 5, centerY, centerX + 5, centerY - 8, centerX + 5, centerY - 2, SSD1306_WHITE);
    display.fillRect(centerX, centerY - 4, size/2 - 5, 8, SSD1306_WHITE);
  }
  // Ramp (on/off ramp)
  else if (direction.indexOf("ramp") >= 0 || direction.indexOf("on ramp") >= 0 || direction.indexOf("off ramp") >= 0) {
    // Ramp icon - curved arrow going up and to the side
    // Base (entering ramp)
    display.fillRect(centerX - 3, centerY + 6, 6, 8, SSD1306_WHITE);
    // Curved path
    if (direction.indexOf("left") >= 0) {
      // Ramp to left
      display.fillTriangle(centerX - 8, centerY - 2, centerX - 12, centerY - 6, centerX - 6, centerY - 1, SSD1306_WHITE);
      display.fillRect(centerX - 12, centerY - 6, 6, 4, SSD1306_WHITE);
    } else if (direction.indexOf("right") >= 0) {
      // Ramp to right
      display.fillTriangle(centerX + 8, centerY - 2, centerX + 12, centerY - 6, centerX + 6, centerY - 1, SSD1306_WHITE);
      display.fillRect(centerX + 6, centerY - 6, 6, 4, SSD1306_WHITE);
    } else {
      // Straight ramp
      display.fillTriangle(centerX, centerY - 8, centerX - 6, centerY - 4, centerX + 6, centerY - 4, SSD1306_WHITE);
      display.fillRect(centerX - 3, centerY - 4, 6, 4, SSD1306_WHITE);
    }
  }
  // Merge
  else if (direction.indexOf("merge") >= 0) {
    // Merge icon - two arrows converging
    // Left arrow
    display.fillTriangle(centerX - 8, centerY - 8, centerX - 2, centerY - 12, centerX - 2, centerY - 6, SSD1306_WHITE);
    display.fillRect(centerX - 2, centerY - 6, 4, 4, SSD1306_WHITE);
    // Right arrow
    display.fillTriangle(centerX + 8, centerY - 8, centerX + 2, centerY - 12, centerX + 2, centerY - 6, SSD1306_WHITE);
    display.fillRect(centerX + 2, centerY - 6, 4, 4, SSD1306_WHITE);
    // Merged arrow going up
    display.fillTriangle(centerX, y + 4, centerX - 8, centerY - 4, centerX - 4, centerY - 4, SSD1306_WHITE);
    display.fillTriangle(centerX, y + 4, centerX + 8, centerY - 4, centerX + 4, centerY - 4, SSD1306_WHITE);
    display.fillRect(centerX - 4, centerY - 4, 8, 8, SSD1306_WHITE);
  }
  // Fork
  else if (direction.indexOf("fork") >= 0) {
    // Fork icon - arrow splitting
    // Base arrow
    display.fillRect(centerX - 4, centerY + 8, 8, 10, SSD1306_WHITE);
    // Left fork
    display.fillTriangle(centerX - 8, centerY - 4, centerX - 12, centerY - 8, centerX - 6, centerY - 2, SSD1306_WHITE);
    display.fillRect(centerX - 12, centerY - 8, 6, 6, SSD1306_WHITE);
    // Right fork
    display.fillTriangle(centerX + 8, centerY - 4, centerX + 12, centerY - 8, centerX + 6, centerY - 2, SSD1306_WHITE);
    display.fillRect(centerX + 6, centerY - 8, 6, 6, SSD1306_WHITE);
  }
  // Keep left/right
  else if (direction.indexOf("keep left") >= 0) {
    // Keep left - arrow with left indicator
    display.fillTriangle(centerX, y, centerX - 10, centerY - 6, centerX - 4, centerY - 6, SSD1306_WHITE);
    display.fillTriangle(centerX, y, centerX + 10, centerY - 6, centerX + 4, centerY - 6, SSD1306_WHITE);
    display.fillRect(centerX - 5, centerY - 6, 10, size/2, SSD1306_WHITE);
    // Left indicator
    display.fillTriangle(x + 2, centerY + 4, x + 8, centerY, x + 8, centerY + 8, SSD1306_WHITE);
  } else if (direction.indexOf("keep right") >= 0) {
    // Keep right - arrow with right indicator
    display.fillTriangle(centerX, y, centerX - 10, centerY - 6, centerX - 4, centerY - 6, SSD1306_WHITE);
    display.fillTriangle(centerX, y, centerX + 10, centerY - 6, centerX + 4, centerY - 6, SSD1306_WHITE);
    display.fillRect(centerX - 5, centerY - 6, 10, size/2, SSD1306_WHITE);
    // Right indicator
    display.fillTriangle(x + size - 2, centerY + 4, x + size - 8, centerY, x + size - 8, centerY + 8, SSD1306_WHITE);
  }
  // Destination/arrival
  else if (direction.indexOf("destination") >= 0 || direction.indexOf("arrive") >= 0 || direction.indexOf("arrival") >= 0) {
    // Destination icon - flag or checkmark
    // Flag pole
    display.fillRect(centerX - 1, y + 4, 2, size - 8, SSD1306_WHITE);
    // Flag
    display.fillTriangle(centerX + 1, y + 4, centerX + 1, y + 12, centerX + 10, y + 8, SSD1306_WHITE);
    // Base
    display.fillRect(centerX - 4, y + size - 4, 8, 4, SSD1306_WHITE);
  }
  // Continue/straight ahead variations
  else if (direction.indexOf("continue") >= 0 || direction.indexOf("straight ahead") >= 0 || direction.indexOf("go straight") >= 0) {
    // Continue straight - same as default but explicit
    display.fillTriangle(centerX, y, centerX - 12, centerY - 8, centerX - 4, centerY - 8, SSD1306_WHITE);
    display.fillTriangle(centerX, y, centerX + 12, centerY - 8, centerX + 4, centerY - 8, SSD1306_WHITE);
    display.fillRect(centerX - 6, centerY - 8, 12, size/2, SSD1306_WHITE);
  }
  // Default: straight/up arrow (↑)
  else {
    // Complete straight/up arrow (↑)
    // Arrow head (pointing up)
    display.fillTriangle(centerX, y, centerX - 12, centerY - 8, centerX - 4, centerY - 8, SSD1306_WHITE);
    display.fillTriangle(centerX, y, centerX + 12, centerY - 8, centerX + 4, centerY - 8, SSD1306_WHITE);
    // Arrow shaft
    display.fillRect(centerX - 6, centerY - 8, 12, size/2, SSD1306_WHITE);
  }
}

void displayNavigation() {
  Navigation nav = chronos.getNavigation();
  
  if (nav.active) {
    // Get direction text for arrow
    String dirText = nav.directions;
    if (dirText.length() == 0) {
      dirText = nav.title;
    }
    
    // Split screen: Arrow on left (0-51, ~40%), Text on right (52-127, ~60%)
    // Vertical divider line
    display.drawLine(51, 0, 51, SCREEN_HEIGHT - 1, SSD1306_WHITE);
    
    // LEFT SIDE (0-51): Arrow centered vertically
    // Arrow size: 40x40, centered in left area
    int arrowX = (51 - 40) / 2; // Center in left 51 pixels
    int arrowY = (SCREEN_HEIGHT - 40) / 2; // Center vertically
    drawNavigationArrow(dirText, arrowX, arrowY);
    
    // RIGHT SIDE (52-127): Text information - use full height with word wrapping
    int rightStartX = 55; // Start text a bit after divider
    int rightWidth = SCREEN_WIDTH - rightStartX; // Available width for text (~73 pixels)
    int lineHeight = 8; // Height per line (text size 1)
    int currentY = 0; // Track current Y position
    
    // Instruction text - can wrap to multiple lines
    if (dirText.length() > 0) {
      display.setTextSize(1);
      String instruction = dirText;
      instruction.toUpperCase();
      // Clean up instruction text
      instruction.replace("TURN ", "");
      instruction.replace(" IN ", " ");
      
      // Word wrap the instruction text
      int maxCharsPerLine = rightWidth / 6; // ~12 chars per line
      int currentPos = 0;
      
      while (currentPos < (int)instruction.length() && currentY < SCREEN_HEIGHT - lineHeight) {
        int charsToShow = min(maxCharsPerLine, (int)instruction.length() - currentPos);
        String line = instruction.substring(currentPos, currentPos + charsToShow);
        
        // Try to break at word boundary
        if (currentPos + charsToShow < (int)instruction.length()) {
          int lastSpace = line.lastIndexOf(' ');
          if (lastSpace > 0) {
            line = line.substring(0, lastSpace);
            charsToShow = lastSpace + 1;
          }
        }
        
        display.setCursor(rightStartX, currentY);
        display.print(line);
        currentY += lineHeight;
        currentPos += charsToShow;
      }
      currentY += 2; // Small spacing after instruction
    }
    
    // Distance to next turn - prominent, can wrap
    if (nav.title.length() > 0 && currentY < SCREEN_HEIGHT - 16) {
      display.setCursor(rightStartX, currentY);
      display.setTextSize(2);
      String dist = nav.title;
      dist.trim();
      // Truncate if too long for one line (size 2 = ~6 chars max)
      int maxDistChars = rightWidth / 12;
      if (dist.length() > maxDistChars) {
        dist = dist.substring(0, maxDistChars);
      }
      display.print(dist);
      currentY += 18; // Size 2 text height + spacing
    }
    
    // Total distance - can wrap to multiple lines
    if (nav.distance.length() > 0 && currentY < SCREEN_HEIGHT - lineHeight) {
      display.setTextSize(1);
      String dist = nav.distance;
      int maxChars = rightWidth / 6;
      
      int currentPos = 0;
      while (currentPos < (int)dist.length() && currentY < SCREEN_HEIGHT - lineHeight) {
        int charsToShow = min(maxChars, (int)dist.length() - currentPos);
        String line = dist.substring(currentPos, currentPos + charsToShow);
        
        // Try to break at word boundary
        if (currentPos + charsToShow < (int)dist.length()) {
          int lastSpace = line.lastIndexOf(' ');
          if (lastSpace > 0) {
            line = line.substring(0, lastSpace);
            charsToShow = lastSpace + 1;
          }
        }
        
        display.setCursor(rightStartX, currentY);
        display.print(line);
        currentY += lineHeight;
        currentPos += charsToShow;
      }
    }
    
    // Duration - can wrap to multiple lines
    if (nav.duration.length() > 0 && currentY < SCREEN_HEIGHT - lineHeight) {
      display.setTextSize(1);
      String dur = nav.duration;
      int maxChars = rightWidth / 6;
      
      int currentPos = 0;
      while (currentPos < (int)dur.length() && currentY < SCREEN_HEIGHT - lineHeight) {
        int charsToShow = min(maxChars, (int)dur.length() - currentPos);
        String line = dur.substring(currentPos, currentPos + charsToShow);
        
        // Try to break at word boundary
        if (currentPos + charsToShow < (int)dur.length()) {
          int lastSpace = line.lastIndexOf(' ');
          if (lastSpace > 0) {
            line = line.substring(0, lastSpace);
            charsToShow = lastSpace + 1;
          }
        }
        
        display.setCursor(rightStartX, currentY);
        display.print(line);
        currentY += lineHeight;
        currentPos += charsToShow;
      }
    }
  } else {
    // No navigation - centered message
    display.setCursor(20, 25);
    display.setTextSize(1);
    display.println("No navigation");
    display.setCursor(30, 35);
    display.println("active");
  }
}

