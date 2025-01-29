#ifndef WIFI_HOTSPOT_H
#define WIFI_HOTSPOT_H

#include <Arduino.h>
#include <vector>
#include <WiFi.h>  // Include WiFi library
#include <set>
#include <string> 

// Shared data
extern std::vector<String> targetSSIDs;
extern const std::vector<String> passwords;  // Declare passwords as extern

// Function declarations
void initializeWiFiHotspot();
void handleWiFiConnections();
void sendDeauthFrame(uint8_t* mac);
bool isMACAuthorized(const String& mac); // Vérifie si une MAC est autorisée
void saveWhitelist(); // Sauvegarde la liste blanche dans SPIFFS
void loadWhitelist(); // Charge la liste blanche depuis SPIFFS
void updateConnectionScore(const String& ssid, bool success, bool unauthorized);  // Declare updateConnectionScore


#endif
