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

std::vector<String> capturedSSIDs; 
std::vector<String> capturedPasswords;

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
String extractSSIDFromPacket(const uint8_t* data, int len) {
    if (len < 36) return ""; // Taille minimale pour un paquet valide
    int ssidLen = data[25];  // Longueur du SSID dans ces paquets spécifiques
    if (ssidLen <= 0 || ssidLen > 32 || 26 + ssidLen > len) return ""; // SSID non valide
    return String((const char*)&data[26], ssidLen); // Extraire le SSID
}

// Fonction pour vérifier si un SSID est dans la liste cible
bool isSSIDTarget(const String& ssid) {
    const char** foundSSIDs = getFoundSSIDs();
    int foundSSIDCount = getFoundSSIDCount();

    for (int i = 0; i < foundSSIDCount; i++) {
        if (ssid == String(foundSSIDs[i])) {
            return true; // SSID trouvé dans la liste cible
        }
    }
    return false;
}

// Fonction pour extraire un mot de passe ou des données supplémentaires
String extractPassword(const uint8_t* data, int len, int offset) {
    String potentialPassword;
    for (int i = offset; i < len; i++) {
        if (isPrintable(data[i])) {
            potentialPassword += (char)data[i];
        } else {
            break; // Fin des caractères imprimables
        }
    }
    return potentialPassword;
}

// Fonction pour traiter les paquets Authentication/Association Request
void processConnectionPacket(const uint8_t* data, int len, uint8_t subType) {
    Serial.printf("Paquet capturé, sous-type : %02X\n", subType);

    // Affichez les données brutes pour valider la structure
    Serial.print("Données brutes du paquet : ");
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X ", data[i]);
    }
    Serial.println();

    // Extraction du SSID uniquement pour Association Request (0x00)
    String ssid;
    if (subType == 0x00) {
        int ssidLen = data[25]; // Longueur du SSID
        if (ssidLen > 0 && ssidLen <= 32 && (26 + ssidLen) <= len) {
            ssid = String((const char*)&data[26], ssidLen); // SSID
        }
    }

    if (!ssid.isEmpty()) {
        Serial.printf("SSID extrait : %s\n", ssid.c_str());
    } else {
        Serial.println("SSID extrait : Aucun");
    }

    // Extraction des données supplémentaires (potentiellement un mot de passe)
    String password;
    if (subType == 0x00) { // Association Request
        int offset = 26 + ssid.length(); // Offset après le SSID
        password = extractPassword(data, len, offset);
    }

    if (!password.isEmpty()) {
        Serial.printf("Mot de passe capturé : %s\n", password.c_str());
    } else {
        Serial.println("Aucun mot de passe capturé.");
    }
}




// Callback pour traiter les paquets capturés
void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return; // Traiter uniquement les paquets de gestion et de données

    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* payload = pkt->payload;
    int len = pkt->rx_ctrl.sig_len;

    // Identifier le type de sous-trame
    uint8_t frameControl = payload[0];
    uint8_t subType = (frameControl >> 4) & 0x0F;

    // Ignorer les Beacon Frames (sous-type 0x08)
    if (subType == 0x08) {
        return; // Ne pas traiter les Beacon Frames
    }

    // Afficher les informations du paquet capturé
    Serial.printf("Paquet capturé, type : %d, sous-type : %02X\n", type, subType);

    // Afficher les données brutes pour analyse
    Serial.print("Données brutes du paquet : ");
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X ", payload[i]);
    }
    Serial.println();
}



// Fonction pour analyser les données utilisateur dans un Data Frame
void processDataFrame(const uint8_t* data, int len) {
    Serial.println("Analyse des données du Data Frame :");

    // Extraire les adresses MAC
    String macSource = extractMAC(data, 10);
    String macDest = extractMAC(data, 4);

    Serial.printf("MAC Source : %s\n", macSource.c_str());
    Serial.printf("MAC Destination : %s\n", macDest.c_str());

    // Rechercher un mot de passe dans les données utilisateur (décodage HTTP)
    Serial.print("Recherche de mot de passe dans les données utilisateur : ");
    int dataOffset = 24; // Offset typique des données utilisateur dans un Data Frame
    for (int i = dataOffset; i < len; i++) {
        if (isPrintable(data[i])) {
            Serial.print((char)data[i]);
        } else {
            Serial.print(".");
        }
    }
    Serial.println();
}

// Fonction pour extraire une adresse MAC
String extractMAC(const uint8_t* data, int offset) {
    char mac[18];
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
            data[offset], data[offset + 1], data[offset + 2],
            data[offset + 3], data[offset + 4], data[offset + 5]);
    return String(mac);
}

// Initialisation du mode promiscuous
void initializePromiscuousMode() {
    Serial.println("Initialisation du mode promiscuous...");
    esp_wifi_set_promiscuous(true); // Activer le mode promiscuous
    esp_wifi_set_promiscuous_rx_cb(snifferCallback); // Définir le callback
    Serial.println("Mode promiscuous activé.");
}

void testPasswords(const char* ssidList[], int ssidCount, const std::vector<String>& passwordList) {
    int passwordCount = passwordList.size();
    bool ssidTested[ssidCount] = {false}; // Track tested SSIDs
    unsigned long totalStartTime = millis(); // Start the total timer

    for (int left = 0, right = passwordCount - 1; left <= right; left++, right--) {
        bool allSSIDsTested = true;

        for (int i = 0; i < ssidCount; ++i) {
            if (ssidTested[i]) continue; // Skip already tested SSIDs

            const char* ssid = ssidList[i];
            Serial.printf("-----------------\n");
            Serial.printf("Testing SSID: %s\n", ssid);

            // Test the left password
            String password = passwordList[left];
            Serial.printf("Testing Password: %s\n", password.c_str());
            
            unsigned long startAttemptTime = millis();
            WiFi.begin(ssid, password.c_str());
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 1000) {
                vTaskDelay(10 / portTICK_PERIOD_MS);  // Non-blocking delay
            }

            connectionResult = (WiFi.status() == WL_CONNECTED);
            if (connectionResult) {
                unsigned long connectionTime = (millis() - startAttemptTime) / 1000;
                Serial.printf("Successfully connected to %s in %lu seconds\n", ssid, connectionTime);
                updateConnectionScore(ssid, true, false); // Update the screen with the connection attempt
                ssidTested[i] = true; // Mark SSID as tested
                WiFi.disconnect(true);
                continue;
            } else {
                Serial.printf("Failed to connect to %s\n", ssid);
                updateConnectionScore(ssid, false, false); // Update the screen with the connection attempt
            }
            WiFi.disconnect(true);
            Serial.printf("-----------------\n");
            
            if (left == right) continue;

            // Test the right password
            password = passwordList[right];
            
            Serial.printf("Testing Password: %s\n", password.c_str());
            
            startAttemptTime = millis();
            WiFi.begin(ssid, password.c_str());
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 1000) {
                vTaskDelay(10 / portTICK_PERIOD_MS);  // Non-blocking delay
            }

            connectionResult = (WiFi.status() == WL_CONNECTED);
            if (connectionResult) {
                unsigned long connectionTime = (millis() - startAttemptTime) / 1000;
                Serial.printf("Successfully connected to %s in %lu seconds\n", ssid, connectionTime);
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

    unsigned long totalTime = (millis() - totalStartTime) / 1000;
    Serial.printf("Total time taken for testing all SSIDs: %lu seconds\n", totalTime);
}
