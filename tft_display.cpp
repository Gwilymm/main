#include "tft_display.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "QRCodeGenerator.h"  // Bibliothèque pour la génération de QR codes

#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_BL   27

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void initializeTFT() {
    SPI.begin(TFT_SCLK, -1, TFT_MOSI); // MISO n'est pas utilisé, on le met à -1
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    delay(1000); 
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    delay(1000); 
    Serial.println("TFT initialized");
}

void displayMessage(const char *message, uint16_t color, uint8_t textSize, int16_t x, int16_t y) {
    tft.setTextColor(color);
    tft.setTextSize(textSize);
    tft.setCursor(x, y);
    tft.print(message);
}

void displayQRCode(const char* text) {
    tft.fillScreen(ILI9341_BLACK);

    // Création d'une instance de QRCode
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)]; // Taille du buffer pour le niveau 3 de correction d'erreur

    // Initialisation du QR code avec le texte
    qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

    // Paramètres d'affichage sur l'écran TFT
    int qrX = 50;            // Position en X du QR code
    int qrY = 50;            // Position en Y du QR code
    int moduleSize = 6;      // Taille de chaque module (point)
    int size = qrcode.size;  // Taille du QR code généré

    // Boucle pour dessiner le QR code sur l'écran TFT
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int color = qrcode_getModule(&qrcode, x, y) ? ILI9341_BLACK : ILI9341_WHITE;
            tft.fillRect(qrX + x * moduleSize, qrY + y * moduleSize, moduleSize, moduleSize, color);
        }
    }
}


