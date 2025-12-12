/*
 * BLE Handler - Manages BLE connection and callbacks
 */

#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include <Arduino.h>
#include <ChronosESP32.h>
#include <Adafruit_SSD1306.h>
#include "display_manager.h"

// Forward declarations
extern ChronosESP32 chronos;
extern Adafruit_SSD1306 display;
extern DisplayMode currentMode;
extern bool displayNeedsUpdate;

// Function declarations
void initBLE();
void onConnection(bool connected);
void onNotification(Notification notification);

#endif // BLE_HANDLER_H

