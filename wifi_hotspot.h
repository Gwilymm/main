#ifndef WIFI_HOTSPOT_H
#define WIFI_HOTSPOT_H

#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <vector>
#include <set>
#include <map>
#include <string>

extern std::vector<std::string> whitelistMAC;
extern std::set<String> loggedUnauthorizedMACs;
extern std::map<String, unsigned long> lastAttemptTime;
extern const unsigned long COOLDOWN_PERIOD;
extern const std::vector<String> passwords;

void initializeWiFiHotspot();
void handleWiFiConnections();

#endif
