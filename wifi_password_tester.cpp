#include <Arduino.h>
#include <WiFi.h>
#include "wifi_password_tester.h"
#include "wifi_hotspot.h"  // Include the wifi_hotspot header to access the password list
#include "tft_display.h"  // Include the tft_display header to update the screen

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

void testWiFiConnection(void* parameter) {
    const char* ssid = ((const char**)parameter)[0];
    const char* password = ((const char**)parameter)[1];

    testInProgress = true;
    Serial.printf("Starting test for SSID: %s with Password: %s\n", ssid, password);

    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 2000) {
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Non-blocking delay
    }

    connectionResult = (WiFi.status() == WL_CONNECTED);
    if (connectionResult) {
        Serial.printf("Successfully connected to %s\n", ssid);
    } else {
        Serial.printf("Failed to connect to %s\n", ssid);
    }

    WiFi.disconnect(true);
    testInProgress = false;
    vTaskDelete(NULL);
}

void testPasswords(const char* ssidList[], int ssidCount, const std::vector<String>& passwordList) {
    bool ssidTested[ssidCount] = {false}; // Track tested SSIDs

    for (const auto& password : passwordList) {
        for (int i = 0; i < ssidCount; ++i) {
            if (ssidTested[i]) continue; // Skip already tested SSIDs

            const char* ssid = ssidList[i];
            Serial.printf("Testing SSID: %s with Password: %s\n", ssid, password.c_str());

            // Prepare parameters for the FreeRTOS task
            const char* params[] = {ssid, password.c_str()};
            testInProgress = true;
            xTaskCreate(testWiFiConnection, "WiFiTestTask", 4096, (void*)params, 1, NULL);

            // Wait for the test to complete
            while (testInProgress) {
                vTaskDelay(100 / portTICK_PERIOD_MS);  // Non-blocking delay
            }

            if (connectionResult) {
                ssidTested[i] = true; // Mark SSID as tested
                updateConnectionScore(ssid, true, false); // Update the screen with the connection attempt
            } else {
                updateConnectionScore(ssid, false, false); // Update the screen with the connection attempt
            }
        }
    }
}
