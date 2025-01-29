#include <Arduino.h>
#include "tft_display.h"
#include "wifi_hotspot.h"
#include "ble_server.h"
#include "wifi_password_tester.h"

// Add this line at the top after includes
extern const std::vector<String> passwords;

void setup() {
    Serial.begin(115200);
    delay(1000);  // Add delay to ensure Serial is ready
    Serial.println("Starting ESP32...");  // Add verification message
    WiFi.mode(WIFI_STA);  // Mode station Wi-Fi pour scanner et tester


    initializeTFT();                        // Initialiser l'écran TFT
    displayQRCode("https://gwilymm.github.io/espHack-security");  // Affiche un QR code

    initializeWiFiHotspot();                // Démarrer le hotspot Wi-Fi
    initializeBLE();  // ✅ Initialisation BLE avant tout

    scanNetworks();
    // Get passwords from wifi_hotspot
    testPasswords(getFoundSSIDs(), getFoundSSIDCount(), passwords); // ✅ Tester les mots de passe après BLE, pour éviter tout conflit réseau

    Serial.println("✅ Setup terminé !");
}

void loop() {
    
    handleWiFiConnections();  // Gérer les connexions Wi-Fi
    delay(100);  // Ajouter un délai pour éviter les boucles rapides
}
