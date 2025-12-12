/*
 * Notification Queue - Manages notification queue and display timing
 */

#include "notification_queue.h"
#include "config.h"

// Notification queue
Notification notificationQueue[MAX_NOTIFICATIONS];
int notificationQueueCount = 0;
int currentNotificationIndex = 0;
unsigned long notificationStartTime = 0;

void initNotificationQueue() {
  notificationQueueCount = 0;
  currentNotificationIndex = 0;
  notificationStartTime = 0;
}

void addNotification(Notification notification) {
  // Add to queue if not full, otherwise shift and add
  if (notificationQueueCount < MAX_NOTIFICATIONS) {
    notificationQueue[notificationQueueCount] = notification;
    notificationQueueCount++;
  } else {
    // Shift queue left and add new notification at end
    for (int i = 0; i < MAX_NOTIFICATIONS - 1; i++) {
      notificationQueue[i] = notificationQueue[i + 1];
    }
    notificationQueue[MAX_NOTIFICATIONS - 1] = notification;
  }
  
  currentNotificationIndex = notificationQueueCount - 1;
  notificationStartTime = millis();
}

void processNotificationQueue(unsigned long currentTime, bool navActive) {
  if (notificationQueueCount == 0) {
    return;
  }
  
  // Use shorter display time if navigation is active
  unsigned long notificationTimeout = navActive ? NOTIFICATION_DISPLAY_TIME_NAV : NOTIFICATION_DISPLAY_TIME;
  
  // Check if current notification has expired
  if ((currentTime - notificationStartTime) >= notificationTimeout) {
    // Remove displayed notification from queue
    for (int i = currentNotificationIndex; i < notificationQueueCount - 1; i++) {
      notificationQueue[i] = notificationQueue[i + 1];
    }
    notificationQueueCount--;
    
    // If more notifications, show next one
    if (notificationQueueCount > 0) {
      currentNotificationIndex = 0;
      notificationStartTime = currentTime;
    } else {
      currentNotificationIndex = 0;
    }
  }
}

bool hasActiveNotification(unsigned long currentTime, bool navActive) {
  if (notificationQueueCount == 0) {
    return false;
  }
  
  unsigned long notificationTimeout = navActive ? NOTIFICATION_DISPLAY_TIME_NAV : NOTIFICATION_DISPLAY_TIME;
  return (currentTime - notificationStartTime) < notificationTimeout;
}

