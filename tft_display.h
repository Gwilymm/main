#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <map>
#include <FS.h>        // Pour le système de fichiers SPIFFS
#include <SPIFFS.h>    // Pour SPIFFS

// Déclaration des fonctions
void initializeTFT();
void displayMessage(const char *message, uint16_t color, uint8_t textSize, int16_t x, int16_t y);
void displayQRCode(const char* text); // Fonction pour afficher le QR code

#endif
