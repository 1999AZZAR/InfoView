/*
 * BLE Handler - Manages BLE connection and callbacks
 */

#include "ble_handler.h"
#include "notification_queue.h"
#include "weather_cache.h"
#include "config.h"

// External objects
extern ChronosESP32 chronos;
extern Adafruit_SSD1306 display;
extern DisplayMode currentMode;
extern bool displayNeedsUpdate;

void initBLE() {
  chronos.setConnectionCallback(onConnection);
  chronos.setNotificationCallback(onNotification);
  chronos.setNotifyBattery(true); // Enable phone battery notifications
  chronos.begin();
}

void onConnection(bool connected) {
  if (connected) {
    chronos.syncRequest(); // Request time sync
  }
  // LED removed for lower power consumption
}

void onNotification(Notification notification) {
  addNotification(notification);
  currentMode = MODE_NOTIFICATION;
  displayNeedsUpdate = true;
}

