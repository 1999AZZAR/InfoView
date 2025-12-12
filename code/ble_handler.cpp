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
    Serial.println("Chronos app connected");
    digitalWrite(LED_PIN, HIGH);
    chronos.syncRequest(); // Request time sync
  } else {
    Serial.println("Chronos app disconnected");
    digitalWrite(LED_PIN, LOW);
  }
}

void onNotification(Notification notification) {
  addNotification(notification);
  currentMode = MODE_NOTIFICATION;
  displayNeedsUpdate = true;
}

