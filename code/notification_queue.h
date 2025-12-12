/*
 * Notification Queue - Manages notification queue and display timing
 */

#ifndef NOTIFICATION_QUEUE_H
#define NOTIFICATION_QUEUE_H

#include <Arduino.h>
#include <ChronosESP32.h>
#include "config.h"

// Notification queue
extern Notification notificationQueue[MAX_NOTIFICATIONS];
extern int notificationQueueCount;
extern int currentNotificationIndex;
extern unsigned long notificationStartTime;

// Function declarations
void initNotificationQueue();
void addNotification(Notification notification);
void processNotificationQueue(unsigned long currentTime, bool navActive);
bool hasActiveNotification(unsigned long currentTime, bool navActive);

#endif // NOTIFICATION_QUEUE_H

