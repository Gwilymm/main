#ifndef WIFI_HOTSPOT_H
#define WIFI_HOTSPOT_H

#include <Arduino.h>
#include <vector>
#include <WiFi.h>  // Include WiFi library

// Shared data
extern std::vector<String> targetSSIDs;
extern const std::vector<String> passwords;  // Declare passwords as extern

// Function declarations
void initializeWiFiHotspot();
bool isMACAuthorized(const String& macAddress);
void handleWiFiConnections();
void scanWiFiNetworks();
void updateConnectionScore(const String& ssid, bool success, bool unauthorized);  // Declare updateConnectionScore

#endif
