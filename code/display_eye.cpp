/*
 * Display Eye - Natural "Almond Shape" & Resting State
 */

 #include "display_eye.h"
 #include "config.h"
 
 // External objects
 extern Adafruit_SSD1306 display;
 
 // --- Tuning Constants ---
 #define EYE_RADIUS 26          // Base white ball size
 #define PUPIL_RADIUS 9         // Slightly smaller pupil for the almond shape
 #define EYELID_RADIUS 70       // Giant masking circle radius (flatter curve)
 
// --- Natural Shape Tuning ---
// These control the "Resting Face" shape. 
// 0 = Fully open (shocked). Higher = More closed (sleepy/relaxed).
#define TOP_LID_RESTING 8      // Top lid covers 8 pixels when "open" (more natural almond)
#define BOTTOM_LID_RESTING 5   // Bottom lid covers 5 pixels when "open"
#define EYELID_CURVE_OFFSET 2  // Additional curve offset for more natural shape
 
 // --- State ---
 static unsigned long lastUpdate = 0;
 static unsigned long nextBlinkTime = 0;
 static unsigned long nextSaccadeTime = 0;
 
 // Blink State
 static bool isBlinking = false;
 static int blinkPhase = 0;     // 0 (Resting) to 100 (Closed)
 static int blinkDirection = 1;
 
 // Gaze State
 static float currentX = 0;
 static float currentY = 0;
 static float targetX = 0;
 static float targetY = 0;
 
 // Helper: Standard map function
 long map_l(long x, long in_min, long in_max, long out_min, long out_max) {
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
 }
 
 void initEye() {
   nextBlinkTime = millis() + 3000;
 }
 
 void displayEye() {
   unsigned long now = millis();
   
   if (now - lastUpdate < 16) return;
   lastUpdate = now;
 
   // ===============================================
   // 1. BEHAVIOR
   // ===============================================
 
   // --- Blink Trigger ---
   if (!isBlinking && now > nextBlinkTime) {
     isBlinking = true;
     blinkPhase = 0;
     blinkDirection = 1;
     nextBlinkTime = now + 3000 + (esp_random() % 5000); 
   }
 
   // --- Look Trigger ---
   if (now > nextSaccadeTime) {
     if ((esp_random() % 100) < 75) {
       // Small movements (Conversational)
       targetX = ((float)(esp_random() % 100) / 40.0) - 1.25; 
       targetY = ((float)(esp_random() % 80) / 40.0) - 1.0;
     } else {
       // Big movements (Looking away)
       targetX = ((float)(esp_random() % 400) / 100.0) - 2.0; 
       targetY = ((float)(esp_random() % 200) / 100.0) - 1.0;
     }
     nextSaccadeTime = now + 800 + (esp_random() % 2500);
   }
 
   // ===============================================
   // 2. PHYSICS
   // ===============================================
 
  if (isBlinking) {
    // Asymmetric blink speeds: faster close, slower open (more natural)
    int closeSpeed = 14; // Faster closing
    int openSpeed = 8;  // Opening speed (as before)
    
    if (blinkDirection == 1) {
      blinkPhase += closeSpeed;
      if (blinkPhase >= 100) {
        blinkPhase = 100;
        blinkDirection = -1;
      }
    } else {
      blinkPhase -= openSpeed;
      if (blinkPhase <= 0) {
        blinkPhase = 0;
        isBlinking = false;
      }
    }
  }
 
   // Smooth Gaze
   currentX += (targetX - currentX) * 0.25;
   currentY += (targetY - currentY) * 0.25;
 
   // ===============================================
   // 3. RENDERER (Almond Shape Logic)
   // ===============================================
   
   int cx = SCREEN_WIDTH / 2;
   int cy = SCREEN_HEIGHT / 2;
   int eyeOffset = 32; 
 
   for(int i=0; i<2; i++) {
     int eyeCX = (i==0) ? cx - eyeOffset : cx + eyeOffset;
     int eyeCY = cy;
 
    // A. Base Eye (White) - Perfect circle for natural eye shape
    display.fillCircle(eyeCX, eyeCY, EYE_RADIUS, SSD1306_WHITE);
    
    // Optional: Add subtle inner corner highlight for more realism
    display.fillCircle(eyeCX - EYE_RADIUS + 3, eyeCY, 2, SSD1306_WHITE);
 
     // B. Pupil (Black)
     // IMPORTANT: We constrain vertical movement more tightly now
     // because the eye is "squinting" slightly in its resting state.
     int limitX = EYE_RADIUS - PUPIL_RADIUS - 2;
     int limitY = EYE_RADIUS - PUPIL_RADIUS - 6; // Stricter Y limit
     
     int pX = eyeCX + (int)(currentX * 6.0);
     int pY = eyeCY + (int)(currentY * 4.0);
     
     if (pX > eyeCX + limitX) pX = eyeCX + limitX;
     if (pX < eyeCX - limitX) pX = eyeCX - limitX;
     if (pY > eyeCY + limitY) pY = eyeCY + limitY;
     if (pY < eyeCY - limitY) pY = eyeCY - limitY;
 
     display.fillCircle(pX, pY, PUPIL_RADIUS, SSD1306_BLACK);
 
     // C. Glint
     display.fillCircle(pX - 3, pY - 3, 3, SSD1306_WHITE);
 
    // D. The Eyelids (Shaping the Eye)
    // We calculate the Y position of the mask circles.
    // Instead of starting "off screen", we start at the RESTING position.

    // --- Upper Lid Calculation ---
    // Open Position: Overlaps by TOP_LID_RESTING pixels for natural almond shape
    // Refined curve positioning for more natural appearance
    int topOpenY = (eyeCY - EYE_RADIUS) - EYELID_RADIUS + TOP_LID_RESTING + EYELID_CURVE_OFFSET;
    // Closed Position: Reaches center for full closure with refined positioning
    int topClosedY = eyeCY - EYELID_RADIUS + 1; // Precise closure at center
    
    // Smooth interpolation for eyelid movement with refined curve
    int currentTopY = map_l(blinkPhase, 0, 100, topOpenY, topClosedY);
    display.fillCircle(eyeCX, currentTopY, EYELID_RADIUS, SSD1306_BLACK);

    // --- Lower Lid Calculation ---
    // Open Position: Overlaps by BOTTOM_LID_RESTING pixels with refined curve
    int botOpenY = (eyeCY + EYE_RADIUS) + EYELID_RADIUS - BOTTOM_LID_RESTING - EYELID_CURVE_OFFSET;
    // Closed Position: Rises to meet upper lid precisely
    int botClosedY = eyeCY + EYELID_RADIUS - 1; // Precise closure at center

    // Lower lid moves less (asymmetric blink - lower lid moves ~35% of upper lid)
    // Refined for more natural movement
    int lowerLidPhase = (blinkPhase * 35) / 100; // Lower lid moves slower, more natural
    int currentBotY = map_l(lowerLidPhase, 0, 100, botOpenY, botClosedY); 
    display.fillCircle(eyeCX, currentBotY, EYELID_RADIUS, SSD1306_BLACK);
 
   }
 }