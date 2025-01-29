#include "ble_server.h"
#include <NimBLEDevice.h>
#include <vector>
#include <algorithm>
#include "wifi_hotspot.h" // Pour synchroniser la liste blanche avec le hotspot
#include "shared_data.h"

// Liste des codes de sécurité autorisés
const std::vector<std::string> securityCodes = {
    "dqihsqlp", "wkdqlqhl", "nywaiwul", "npvnqjpn",
    "pevtmobv", "hnwfeblw", "swynfvrl", "akgmnvec",
    "fhzeamrx", "ozxgzixh"
};

// Remove any definitions of whitelistMAC, loggedUnauthorizedMACs, etc.
// They are now defined in wifi_hotspot.cpp

const char* deviceName = "ESP32_BLE_DEVICE";
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pNotifyCharacteristic = nullptr;

// Vérifie si un code de sécurité est valide
bool isCodeValid(const std::string& code) {
    return std::find(securityCodes.begin(), securityCodes.end(), code) != securityCodes.end();
}

// Supprime les espaces inutiles d'une chaîne
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    if (first == std::string::npos || last == std::string::npos)
        return ""; // Retourne une chaîne vide si str est uniquement composé d'espaces
    return str.substr(first, (last - first + 1));
}

// Vérifie si une adresse MAC est déjà autorisée
bool isMACAuthorized(const std::string& mac) {
    return std::find(whitelistMAC.begin(), whitelistMAC.end(), mac) != whitelistMAC.end();
}

// Ajoute une MAC autorisée si le code de sécurité est valide
void authorizeMAC(const std::string& macAddress, const std::string& securityCode) {
    std::string trimmedMAC = trim(macAddress);

    if (!isCodeValid(securityCode)) {
        Serial.println("Code de sécurité invalide.");
        if (pNotifyCharacteristic != nullptr) {
            pNotifyCharacteristic->setValue("INVALID CODE");
            pNotifyCharacteristic->notify();
        }
        return;
    }

    if (isMACAuthorized(trimmedMAC)) {
        Serial.println("MAC déjà autorisée.");
        if (pNotifyCharacteristic != nullptr) {
            pNotifyCharacteristic->setValue("MAC ALREADY AUTHORIZED");
            pNotifyCharacteristic->notify();
        }
        return;
    }

    whitelistMAC.push_back(trimmedMAC);
    saveWhitelist(); // Synchronise avec le système de gestion WiFi
    Serial.printf("MAC autorisée : %s\n", trimmedMAC.c_str());

    if (pNotifyCharacteristic != nullptr) {
        pNotifyCharacteristic->setValue("MAC AUTHORIZED");
        pNotifyCharacteristic->notify();
    }
}

// Callbacks pour la caractéristique BLE
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (!value.empty()) {
            Serial.printf("Données reçues via BLE : %s\n", value.c_str());

            // Attend une chaîne au format "MAC:<adresse MAC>, CODE:<code>"
            size_t macPos = value.find("MAC:");
            size_t codePos = value.find("CODE:");

            if (macPos != std::string::npos && codePos != std::string::npos) {
                std::string macAddress = value.substr(macPos + 4, codePos - macPos - 5);
                std::string securityCode = value.substr(codePos + 5);

                // Traite la validation
                authorizeMAC(macAddress, securityCode);
            } else {
                Serial.println("Format invalide reçu.");
                if (pNotifyCharacteristic != nullptr) {
                    pNotifyCharacteristic->setValue("INVALID FORMAT");
                    pNotifyCharacteristic->notify();
                }
            }
        }
    }
};

// Initialise le serveur BLE
void initializeBLE() {
    NimBLEDevice::init(deviceName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    // Créer le serveur BLE
    pServer = NimBLEDevice::createServer();

    // Créer le service BLE
    NimBLEService* pService = pServer->createService("12345678-1234-5678-1234-56789ABCDEF0");

    // Créer une caractéristique pour recevoir les commandes
    NimBLECharacteristic* pWriteCharacteristic = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789ABCDEF1",
        NIMBLE_PROPERTY::WRITE
    );
    pWriteCharacteristic->setCallbacks(new CharacteristicCallbacks());

    // Créer une caractéristique pour envoyer des notifications
    pNotifyCharacteristic = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789ABCDEF2",
        NIMBLE_PROPERTY::NOTIFY
    );

    // Démarrer le service et la publicité BLE
    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();

    Serial.println("BLE Server initialized and advertising.");
}
