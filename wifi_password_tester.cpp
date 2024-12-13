#include <Arduino.h>
#include <WiFi.h>
#include "wifi_password_tester.h"
#include "wifi_hotspot.h"  // Include the wifi_hotspot header to access the password list
#include "tft_display.h"  // Include the tft_display header to update the screen
#include <esp_wifi.h>  // Include the ESP WiFi library for debugging

#define MAX_SSIDS 50
static const char* foundSSIDs[MAX_SSIDS];
static int foundSSIDCount = 0;

bool testInProgress = false;  // Global variable to indicate if a test is in progress
bool connectionResult = false;  // Global variable to store the connection result

void initializePasswordTester() {
    // Any initialization code for the password tester
}

void scanNetworks() {
    Serial.println("Scanning for networks...");
    int n = WiFi.scanNetworks();
    foundSSIDCount = 0;
    if (n == WIFI_SCAN_FAILED) {
        Serial.println("WiFi scan failed");
    } else if (n == 0) {
        Serial.println("No networks found");
    } else if (n < 0) {
        Serial.printf("WiFi scan error: %d\n", n);
    } else {
        Serial.printf("%d networks found:\n", n);
        for (int i = 0; i < n && foundSSIDCount < MAX_SSIDS; ++i) {
            String ssid = WiFi.SSID(i);
            if (ssid.startsWith("wifi_m2dfs")) {
                foundSSIDs[foundSSIDCount++] = strdup(ssid.c_str());
                Serial.printf("%d: %s (%d)\n", foundSSIDCount, ssid.c_str(), WiFi.RSSI(i));
            }
        }
    }
    Serial.println("Scan complete");
}

const char** getFoundSSIDs() {
    return foundSSIDs;
}

int getFoundSSIDCount() {
    return foundSSIDCount;
}

void testPasswords(const char* ssidList[], int ssidCount, const std::vector<String>& passwordList) {
    int passwordCount = passwordList.size();
    bool ssidTested[ssidCount] = {false}; // Track tested SSIDs



    for (int left = 0, right = passwordCount - 1; left <= right; left++, right--) {
        bool allSSIDsTested = true;

        for (int i = 0; i < ssidCount; ++i) {
            if (ssidTested[i]) continue; // Skip already tested SSIDs

            const char* ssid = ssidList[i];
            Serial.printf("Testing SSID: %s\n", ssid);

            // Test the left password
            String password = passwordList[left];
            Serial.printf("Testing Password: %s\n", password.c_str());
            WiFi.begin(ssid, password.c_str());
            unsigned long startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 1000) {
                vTaskDelay(10 / portTICK_PERIOD_MS);  // Non-blocking delay
            }

            connectionResult = (WiFi.status() == WL_CONNECTED);
            if (connectionResult) {
                Serial.printf("Successfully connected to %s\n", ssid);
                updateConnectionScore(ssid, true, false); // Update the screen with the connection attempt
                ssidTested[i] = true; // Mark SSID as tested
                WiFi.disconnect(true);
                continue;
            } else {
                Serial.printf("Failed to connect to %s\n", ssid);
                updateConnectionScore(ssid, false, false); // Update the screen with the connection attempt
            }
            WiFi.disconnect(true);

            if (left == right) continue;

            // Test the right password
            password = passwordList[right];
            Serial.printf("Testing Password: %s\n", password.c_str());
            WiFi.begin(ssid, password.c_str());
            startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 1000) {
                vTaskDelay(10 / portTICK_PERIOD_MS);  // Non-blocking delay
            }

            connectionResult = (WiFi.status() == WL_CONNECTED);
            if (connectionResult) {
                Serial.printf("Successfully connected to %s\n", ssid);
                updateConnectionScore(ssid, true, false); // Update the screen with the connection attempt
                ssidTested[i] = true; // Mark SSID as tested
                WiFi.disconnect(true);
            } else {
                Serial.printf("Failed to connect to %s\n", ssid);
                updateConnectionScore(ssid, false, false); // Update the screen with the connection attempt
            }
            WiFi.disconnect(true);
        }

        // Check if all SSIDs have been tested
        for (int i = 0; i < ssidCount; ++i) {
            if (!ssidTested[i]) {
                allSSIDsTested = false;
                break;
            }
        }

        if (allSSIDsTested) {
            Serial.println("All SSIDs have been tested.");
            break;
        }
    }
}
