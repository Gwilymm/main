#include <Arduino.h>
#include "tft_display.h"
#include "wifi_hotspot.h"
#include "ble_server.h"
#include "http_server.h"  // Incluez le fichier d'en-tête pour le serveur HTTP

void setup() {
    Serial.begin(115200);

    initializeTFT();                        // Initialise l'écran TFT

    displayQRCode("https://gwilymm.github.io/espHack-security");    // Affiche le QR code
    displayMessage("SSID: WIFI_M2DFS_MARIN", ILI9341_GREEN, 2, 10, 10);

    initializeWiFiHotspot();                // Initialise le point d'accès WiFi avec un mot de passe aléatoire
    initializeBLE();                        // Initialise le BLE pour les fonctions de validation
    initializeHttpServer();                 // Initialise le serveur HTTP
}

void loop() {
    handleWiFiConnections();                // Traite les connexions WiFi et vérifie les adresses MAC autorisées
    server.handleClient();                  // Gère les requêtes entrantes pour le serveur HTTP
    delay(100);
}
