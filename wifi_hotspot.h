#ifndef WIFI_HOTSPOT_H
#define WIFI_HOTSPOT_H

#include <Arduino.h>  // Ajout de cette ligne pour la définition de String

void initializeWiFiHotspot();
bool isMACAuthorized(const String& macAddress);
void handleWiFiConnections();

#endif
