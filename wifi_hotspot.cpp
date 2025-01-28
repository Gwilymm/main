#include "wifi_hotspot.h"
#include "shared_data.h"
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <Arduino.h>

// Define the shared variables in only one source file
std::vector<std::string> whitelistMAC;
std::set<String> loggedUnauthorizedMACs;
std::map<String, unsigned long> lastAttemptTime;
const unsigned long COOLDOWN_PERIOD = 1000;

// Configuration du hotspot
const char* ssid_ap = "WIFI_M2DFS_MARIN";
String chosenPassword;
const std::vector<String> passwords = {
    "kxzvaajn", "nulebozl", "sylmukyv", "fzxhzodw", "mnhfwylw",
    "xhnvtwdd", "jrulvtsp", "ochdamfa", "plaluafr", "ovnkkilc", "oizuxgas", "tjrdbrav", "pklzocye", "vtapalxp", "aaaaaaaa"
};

// Charger un mot de passe aléatoire
String loadRandomPassword() {
    if (passwords.size() > 0) {
        randomSeed(esp_random()); // Initialiser le générateur aléatoire
        int index = random(0, passwords.size());
        return passwords[index];
    }
    return "defaultpass"; // Retourne un mot de passe par défaut
}

// Sauvegarder la liste blanche dans SPIFFS
void saveWhitelist() {
    File file = SPIFFS.open("/whitelist.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Erreur : Impossible de sauvegarder la liste blanche.");
        return;
    }
    for (const auto& mac : whitelistMAC) {
        file.println(mac.c_str());
    }
    file.close();
    Serial.println("Liste blanche sauvegardée.");
}

// Charger la liste blanche depuis SPIFFS
void loadWhitelist() {
    File file = SPIFFS.open("/whitelist.txt", FILE_READ);
    if (!file) {
        Serial.println("Erreur : Impossible de charger la liste blanche.");
        return;
    }
    whitelistMAC.clear();
    while (file.available()) {
        whitelistMAC.push_back(file.readStringUntil('\n').c_str());
    }
    file.close();
    Serial.println("Liste blanche chargée.");
}

// Vérifie si une adresse MAC est autorisée
bool isMACAuthorized(const String& mac) {
    std::string macStd = std::string(mac.c_str());
    return std::find(whitelistMAC.begin(), whitelistMAC.end(), macStd) != whitelistMAC.end();
}

// Initialiser le point d'accès WiFi
void initializeWiFiHotspot() {
    SPIFFS.begin(true); // Initialiser SPIFFS pour la persistance
    loadWhitelist(); // Charger la liste blanche au démarrage

    chosenPassword = loadRandomPassword();
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid_ap, chosenPassword.c_str());
    IPAddress IP_AP = WiFi.softAPIP();

    Serial.println("WiFi Hotspot Initialized");
    Serial.printf("SSID: %s\n", ssid_ap);
    Serial.printf("Password: %s\n", chosenPassword.c_str());
    Serial.printf("AP IP Address: %s\n", IP_AP.toString().c_str());
}

// Envoyer une trame de désauthentification
void sendDeauthFrame(uint8_t* mac) {
    uint8_t YOUR_AP_MAC[6];
    WiFi.softAPmacAddress().toCharArray((char*)YOUR_AP_MAC, 6);

    uint8_t deauth_frame[26] = {
        0xC0, 0x00, // Frame Control
        0x00, 0x00, // Duration
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], // Destination MAC
        YOUR_AP_MAC[0], YOUR_AP_MAC[1], YOUR_AP_MAC[2], YOUR_AP_MAC[3], YOUR_AP_MAC[4], YOUR_AP_MAC[5], // Source MAC
        YOUR_AP_MAC[0], YOUR_AP_MAC[1], YOUR_AP_MAC[2], YOUR_AP_MAC[3], YOUR_AP_MAC[4], YOUR_AP_MAC[5], // BSSID
        0x00, 0x00, // Sequence Control
        0x01, 0x00  // Reason Code
    };
    if (!mac) {
        Serial.println("Erreur : Adresse MAC client invalide.");
        return;
    }

    if (sizeof(deauth_frame) != 26) {
        Serial.println("Erreur : Trame mal formée.");
        return;
    }

    esp_err_t err = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame, sizeof(deauth_frame), false);
        if (err == ESP_ERR_WIFI_NOT_INIT) {
            Serial.println("Erreur : Wi-Fi non initialisé.");
        } else if (err == ESP_ERR_INVALID_ARG) {
            Serial.println("Erreur : Arguments invalides pour esp_wifi_80211_tx.");
        } else if (err == ESP_ERR_WIFI_IF) {
            Serial.println("Erreur : Interface Wi-Fi incorrecte.");
        } else if (err == ESP_OK) {
            Serial.println("Trame de désauthentification envoyée avec succès.");
        } else {
            Serial.printf("Erreur inattendue : %s\n", esp_err_to_name(err));
        }

}

// Gérer les connexions WiFi
void handleWiFiConnections() {
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);

    unsigned long currentMillis = millis();
    for (int i = 0; i < stationList.num; i++) {
        wifi_sta_info_t station = stationList.sta[i];
        String mac = String(station.mac[0], HEX) + ":" + 
                     String(station.mac[1], HEX) + ":" + 
                     String(station.mac[2], HEX) + ":" + 
                     String(station.mac[3], HEX) + ":" + 
                     String(station.mac[4], HEX) + ":" + 
                     String(station.mac[5], HEX);
        mac.toUpperCase();

        if (isMACAuthorized(mac)) {
            Serial.printf("Appareil autorisé : %s\n", mac.c_str());
        } else {
            if (currentMillis - lastAttemptTime[mac] > COOLDOWN_PERIOD) {
                lastAttemptTime[mac] = currentMillis;
                Serial.printf("Appareil non autorisé : %s\n", mac.c_str());
                sendDeauthFrame(station.mac);
            }
        }
    }
}