#include <Arduino.h>
#include "tft_display.h"
#include "wifi_hotspot.h"
#include "ble_server.h"
#include "wifi_password_tester.h"  // Include the new header file

void testPasswordsTask(void* parameter) {
    const char** ssidList = getFoundSSIDs();
    int ssidCount = getFoundSSIDCount();
    testPasswords(ssidList, ssidCount, passwords);  // Test passwords for SSIDs
    vTaskDelete(NULL);  // Delete the task once done
}

void handleWiFiConnectionsTask(void* parameter) {
    while (true) {
        handleWiFiConnections();  // Handle WiFi connections
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Non-blocking delay
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);  // Ensure WiFi is in STA mode at the start

    initializeTFT();                        // Initialise l'écran TFT

    displayQRCode("https://gwilymm.github.io/espHack-security");    // Affiche le QR code
    
    initializeWiFiHotspot();                // Initialize both AP and STA modes

    initializeBLE();                        // Initialise le BLE pour les fonctions de validation
    
    scanNetworks();                         // Scan and list networks
    initializePromiscuousMode();            // Initialize promiscuous mode
    //initializePasswordTester();            // Initialize the password tester

    // Create FreeRTOS tasks
    //xTaskCreate(testPasswordsTask, "TestPasswordsTask", 4096, NULL, 1, NULL);
    //xTaskCreate(handleWiFiConnectionsTask, "HandleWiFiConnectionsTask", 4096, NULL, 1, NULL);

    Serial.println("Setup completed");
}

void loop() {
    // Empty loop as tasks are handled by FreeRTOS
}
