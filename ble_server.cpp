#include "ble_server.h"
#include <NimBLEDevice.h>
#include "tft_display.h"
#include <vector>
#include <WiFi.h>


// Liste des codes de sécurité autorisés
const std::vector<std::string> securityCodes = {
    "dqihsqlp", "wkdqlqhl", "nywaiwul", "npvnqjpn",
    "pevtmobv", "hnwfeblw", "swynfvrl", "akgmnvec",
    "fhzeamrx", "ozxgzixh"
};

// Liste blanche des adresses MAC autorisées
std::vector<std::string> whitelistMAC;

const char* deviceName = "ESP32_BLE_DEVICE";
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCharacteristic = nullptr;
NimBLECharacteristic* pNotifyCharacteristic = nullptr;
bool validationReceived = false;

// Vérifie si un code de sécurité est valide
bool isCodeValid(const std::string& code) {
    return std::find(securityCodes.begin(), securityCodes.end(), code) != securityCodes.end();
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    if (first == std::string::npos || last == std::string::npos)
        return ""; // Retourne une chaîne vide si str est uniquement composé d'espaces
    return str.substr(first, (last - first + 1));
}


// Autoriser une adresse MAC avec un code de sécurité valide et envoyer la réponse via Bluetooth
void authorizeMAC(const std::string& macAddress, const std::string& securityCode) {
    std::string trimmedMAC = trim(macAddress);  // Supprime les espaces inutiles

    if (isCodeValid(securityCode)) {
        // Vérifier si l'adresse MAC sans espaces est déjà dans la liste
        if (std::find(whitelistMAC.begin(), whitelistMAC.end(), trimmedMAC) == whitelistMAC.end()) {
            whitelistMAC.push_back(trimmedMAC);  // Ajoute l'adresse MAC nettoyée
            Serial.print("MAC autorisée : ");
            Serial.println(trimmedMAC.c_str());

            // Envoyer une réponse de succès via Bluetooth
            if (pNotifyCharacteristic != nullptr) {
                pNotifyCharacteristic->setValue("MAC AUTHORIZED");
                pNotifyCharacteristic->notify();
            }
        } else {
            Serial.println("MAC déjà autorisée.");
        }
    } else {
        Serial.println("Code de sécurité invalide");

        // Envoyer une réponse d'échec via Bluetooth
        if (pNotifyCharacteristic != nullptr) {
            pNotifyCharacteristic->setValue("INVALID CODE");
            pNotifyCharacteristic->notify();
        }
    }
}


// Callbacks pour le serveur Bluetooth
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("DEVICE CONNECTED");
        
    }

    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("DEVICE DISCONNECTED");
        
    }
};

// Callbacks pour la caractéristique de validation
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (!value.empty()) {
            Serial.print("DATA RECEIVED: ");
            Serial.println(value.c_str());

            // Attend une chaîne "MAC:<adresse MAC>, CODE:<code>"
            size_t macPos = value.find("MAC:");
            size_t codePos = value.find("CODE:");

            if (macPos != std::string::npos && codePos != std::string::npos) {
                std::string macAddress = value.substr(macPos + 4, codePos - macPos - 5);
                std::string securityCode = value.substr(codePos + 5);

                // Affiche l'adresse MAC reçue
                Serial.print("Adresse MAC reçue : ");
                Serial.println(macAddress.c_str());

                // Autorise l'adresse MAC avec le code de sécurité et envoie la réponse
                authorizeMAC(macAddress, securityCode);
            }
        }
    }
};

// Initialise le serveur BLE sans le mot de passe
void initializeBLE() {
    // Affiche l'adresse MAC de l'ESP32 dans le moniteur série
    String espMac = WiFi.macAddress();
    Serial.print("Adresse MAC de l'ESP32 (STA): ");
    Serial.println(espMac);
    
    String espMacAP = WiFi.softAPmacAddress();
    Serial.print("Adresse MAC de l'ESP32 (AP): ");
    Serial.println(espMacAP);

    NimBLEDevice::init(deviceName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Utilisation d'UUID de 128 bits pour éviter les conflits
    NimBLEService* pService = pServer->createService("12345678-1234-5678-1234-56789ABCDEF0");

    // Créer une caractéristique pour recevoir des commandes
    pCharacteristic = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789ABCDEF1",
        NIMBLE_PROPERTY::WRITE
    );
    pCharacteristic->setCallbacks(new CharacteristicCallbacks());

    // Créer une caractéristique pour envoyer des notifications
    pNotifyCharacteristic = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789ABCDEF2",
        NIMBLE_PROPERTY::NOTIFY
    );

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->setScanResponseData();
    pAdvertising->start();

    Serial.println("BLE initialized and advertising started");
}

bool checkValidationReceived() {
    return validationReceived;
}
