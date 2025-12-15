/*
 * Display Navigation - Comprehensive Bitmap Logic
 */

 #include "display_navigation.h"
 #include "config.h"
 #include "nav_icons.h" 
 
 // External objects
 extern Adafruit_SSD1306 display;
 extern ChronosESP32 chronos;
 extern ESP32Time rtc;
 
 // Helper function to select and draw the correct bitmap
 void drawNavigationIcon(String direction, int x, int y) {
   direction.toLowerCase();
   
   const unsigned char* bitmapPtr = nav_straight_bits; // Default fallback
 
   // --- COMPREHENSIVE SELECTION LOGIC ---
 
   // 1. Roundabouts
   if (direction.indexOf("roundabout") >= 0 || direction.indexOf("traffic circle") >= 0 || direction.indexOf("rotary") >= 0) {
     if (direction.indexOf("left") >= 0 || direction.indexOf("1") >= 0 || direction.indexOf("first") >= 0) {
        bitmapPtr = nav_round_left_bits;
     } else if (direction.indexOf("right") >= 0 || direction.indexOf("3") >= 0 || direction.indexOf("third") >= 0) {
        bitmapPtr = nav_round_right_bits;
     } else {
        bitmapPtr = nav_round_straight_bits; // 2nd exit / straight
     }
   }
   // 2. Sharp Turns
   else if (direction.indexOf("sharp left") >= 0 || direction.indexOf("hard left") >= 0) {
     bitmapPtr = nav_sharp_left_bits;
   } 
   else if (direction.indexOf("sharp right") >= 0 || direction.indexOf("hard right") >= 0) {
     bitmapPtr = nav_sharp_right_bits;
   }
   // 3. U-Turns
   else if (direction.indexOf("u-turn") >= 0 || direction.indexOf("uturn") >= 0 || direction.indexOf("make a u-turn") >= 0) {
     bitmapPtr = nav_uturn_bits;
   }
   // 4. Slight Turns / Bear
   else if (direction.indexOf("slight left") >= 0 || direction.indexOf("bear left") >= 0) {
     bitmapPtr = nav_slight_left_bits;
   }
   else if (direction.indexOf("slight right") >= 0 || direction.indexOf("bear right") >= 0) {
     bitmapPtr = nav_slight_right_bits;
   }
   // 5. Forks
   else if (direction.indexOf("fork") >= 0) {
     if (direction.indexOf("left") >= 0) {
       bitmapPtr = nav_fork_left_bits;
     } else {
       bitmapPtr = nav_fork_right_bits;
     }
   }
   // 6. Ramps & Merges
   else if (direction.indexOf("ramp") >= 0 || direction.indexOf("slip road") >= 0 || direction.indexOf("merge") >= 0 || direction.indexOf("join") >= 0) {
     // For now, mapping all ramps/merges to the Merge icon (converging)
     // You can differentiate left/right ramp specifically if you draw mirrored merge icons, 
     // but often a single "Merge" icon suffices for 1-bit displays.
     if (direction.indexOf("left") >= 0) {
        bitmapPtr = nav_slight_left_bits; // Ramp left often looks like slight left
     } else if (direction.indexOf("right") >= 0) {
        bitmapPtr = nav_slight_right_bits;
     } else {
        bitmapPtr = nav_merge_bits;
     }
   }
   // 7. Keep Left / Right
   else if (direction.indexOf("keep left") >= 0) {
     bitmapPtr = nav_keep_left_bits;
   }
   else if (direction.indexOf("keep right") >= 0) {
     // Reuse Slight Right or Fork Right if you don't have a dedicated "Keep Right" icon
     // or mirror the keep_left logic. 
     bitmapPtr = nav_slight_right_bits; 
   }
   // 8. Regular Turns (Standard)
   else if (direction.indexOf("left") >= 0 || direction.indexOf("turn left") >= 0) {
     bitmapPtr = nav_left_bits;
   } 
   else if (direction.indexOf("right") >= 0 || direction.indexOf("turn right") >= 0) {
     bitmapPtr = nav_right_bits;
   }
   // 9. Destination
   else if (direction.indexOf("destination") >= 0 || direction.indexOf("arrive") >= 0 || direction.indexOf("reached") >= 0) {
     bitmapPtr = nav_dest_bits;
   }
   // Default is straight
 
   // Draw the selected 32x32 bitmap
   display.drawBitmap(x, y, bitmapPtr, ICON_W, ICON_H, SSD1306_WHITE);
 }
 
 void displayNavigation() {
   Navigation nav = chronos.getNavigation();
   
   if (nav.active) {
     String dirText = nav.directions;
     if (dirText.length() == 0) dirText = nav.title;
     
     // --- LAYOUT ---
     int leftW = 51;
     display.drawLine(leftW, 0, leftW, SCREEN_HEIGHT - 1, SSD1306_WHITE);
     
     // LEFT PANEL
     
     // 1. Time
     display.setTextSize(1);
     int hour = chronos.getHourC();
     int minute = rtc.getMinute();
     char timeBuf[6];
     snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
     display.setCursor((leftW - 30)/2, 2);
     display.print(timeBuf);
     
     // 2. Icon
     int iconX = (leftW - ICON_W) / 2;
     int iconY = 14; 
     drawNavigationIcon(dirText, iconX, iconY);
     
     // 3. ETA
     String dur = nav.duration;
     dur.replace(" mins", "m");
     dur.replace(" min", "m");
     dur.replace(" hours", "h");
     dur.replace(" hour", "h");
     if(dur.length() > 7) dur = dur.substring(0, 7);
     int durX = (leftW - (dur.length() * 6)) / 2;
     display.setCursor(durX, SCREEN_HEIGHT - 9);
     display.print(dur);
 
     // RIGHT PANEL
     int rightX = leftW + 4; 
     int rightW = SCREEN_WIDTH - rightX;
     int currentY = 0;
     
     // 1. Distance
     if (nav.title.length() > 0) {
       display.setTextSize(2);
       display.setCursor(rightX, currentY + 2);
       String dist = nav.title;
       if(dist.length() > 6) dist = dist.substring(0,6);
       display.println(dist);
       currentY += 18; 
     } else {
        currentY += 2;
     }
     
     // 2. Instructions
     display.setTextSize(1);
     String instr = dirText;
     instr.replace("TURN ", ""); 
     instr.replace("Turn ", "");
     instr.replace("In ", "");
     instr.replace("Continue", "Cont.");
     
     // Word Wrap
     int lineLenChars = rightW / 6; 
     int cursor = 0;
     display.setCursor(rightX, currentY);
     
     while(cursor < instr.length()) {
       int chunkLen = min((int)instr.length() - cursor, lineLenChars);
       int nextSpace = -1;
       if(cursor + chunkLen < instr.length()) {
          nextSpace = instr.lastIndexOf(' ', cursor + chunkLen);
       }
       if(nextSpace > cursor) {
          display.println(instr.substring(cursor, nextSpace));
          cursor = nextSpace + 1;
       } else {
          display.println(instr.substring(cursor, cursor + chunkLen));
          cursor += chunkLen;
       }
       if(display.getCursorY() > SCREEN_HEIGHT - 8) break; 
       display.setCursor(rightX, display.getCursorY());
     }
     
   } else {
     // Idle
     display.setTextSize(1);
     display.setCursor(30, 25);
     display.println("Ready for");
     display.setCursor(34, 35);
     display.println("Navigation");
   }
 }