#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <map>
#include <FS.h>        // Pour le système de fichiers SPIFFS
#include <SPIFFS.h>    // Pour SPIFFS

// Structure pour stocker les informations de connexion
struct ConnectionInfo {
    int attempts;
    bool success;
};

// Déclaration des fonctions
extern const char* ssid_ap;
extern String chosenPassword;
extern std::map<String, ConnectionInfo> connectionScores;  // Declare connectionScores as external

void displayMACList();
void initializeTFT();
void displayMessage(const char *message, uint16_t color, uint8_t textSize, int16_t x, int16_t y);
void displayQRCode(const char* text); // Fonction pour afficher le QR code
void alternateScreens();
void updateMACList(const String& mac, unsigned long attemptTime);
void displaySuccessConnections();

#endif
