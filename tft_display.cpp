#include "tft_display.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "QRCodeGenerator.h"  // Bibliothèque pour la génération de QR codes
#include "wifi_hotspot.h"

#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_BL   27

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

std::map<String, unsigned long> macList;
std::map<String, ConnectionInfo> connectionScores;  // Define connectionScores map
unsigned long lastScreenSwitch = 0;
unsigned long lastPageSwitch = 0;
bool showQRCodeScreen = true;
bool showSuccessScreen = false; // Add a flag for the success screen
bool showMACListScreen = false; // Add a flag for the MAC list screen
int currentPage = 0;
const int entriesPerPage = 5;

void screenTask(void *pvParameters) {
    while (true) {
        alternateScreens();
        delay(10000); // Adjust the delay to 10 seconds
    }
}

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

    // Create a task for screen updates
    xTaskCreate(screenTask, "Screen Task", 4096, NULL, 1, NULL);
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

void alternateScreens() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastScreenSwitch >= 10000) { // Switch screens every 10 seconds
        if (showQRCodeScreen) {
            showQRCodeScreen = false;
            showSuccessScreen = true;
            showMACListScreen = false;
        } else if (showSuccessScreen) {
            showQRCodeScreen = false;
            showSuccessScreen = false;
            showMACListScreen = true;
        } else if (showMACListScreen) {
            showQRCodeScreen = true;
            showSuccessScreen = false;
            showMACListScreen = false;
        }
        lastScreenSwitch = currentMillis;
        tft.fillScreen(ILI9341_BLACK); // Clear the screen only when switching
    }

    if (showQRCodeScreen) {
        String qrCodeText = "WIFI:S:" + String(ssid_ap) + ";T:WPA;P:" + chosenPassword + ";;";
        displayQRCode(qrCodeText.c_str());
        displayMessage(ssid_ap, ILI9341_WHITE, 2, 10, 10);
    } else if (showSuccessScreen) {
        displaySuccessConnections();
    } else if (showMACListScreen) {
        displayMACList();
    }
}

void displayMACList() {
    unsigned long currentMillis = millis();

    int totalPages = (macList.size() + entriesPerPage - 1) / entriesPerPage;
    if (totalPages > 1 && currentMillis - lastPageSwitch >= 10000) { // Switch pages every 10 seconds if there are multiple pages
        currentPage++;
        lastPageSwitch = currentMillis;
    }

    if (currentPage >= totalPages) {
        currentPage = 0;
    }

    tft.fillScreen(ILI9341_BLACK); // Clear the screen only when switching pages
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.print("MAC Address List");

    int start = currentPage * entriesPerPage;
    int end = start + entriesPerPage;
    int y = 40;

    auto it = macList.begin();
    std::advance(it, start);

    for (int i = start; i < end && it != macList.end(); ++i, ++it) {
        tft.setCursor(10, y);
        tft.print(it->first);
        tft.setCursor(200, y);
        if (connectionScores.find(it->first) != connectionScores.end()) {
            tft.print("Attempts: " + String(connectionScores[it->first].attempts) + " Success: " + (connectionScores[it->first].success ? "Yes" : "No"));
        } else {
            tft.print("No data");
        }
        y += 20;
    }
}

void displaySuccessConnections() {
    tft.fillScreen(ILI9341_BLACK); // Clear the screen
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.print("Successful Connections");

    int y = 40;
    for (const auto& entry : connectionScores) {
        if (entry.second.success) {
            tft.setCursor(10, y);
            tft.print(entry.first);
            tft.setCursor(200, y);
            tft.print("Attempts: " + String(entry.second.attempts));
            y += 20;
        }
    }
}

void updateMACList(const String& mac, unsigned long attemptTime) {
    macList[mac] = attemptTime;
}

void updateConnectionScore(const String& ssid, bool success, bool unauthorized) {
    if (connectionScores.find(ssid) == connectionScores.end()) {
        connectionScores[ssid] = {0, false};
    }
    connectionScores[ssid].attempts++;
    if (success) {
        connectionScores[ssid].success = true;
    }
    // Update the screen with the connection attempt
    displayMACList();
}


