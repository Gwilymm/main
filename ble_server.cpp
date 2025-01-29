#include "ble_server.h"
#include <NimBLEDevice.h>
#include <vector>
#include <algorithm>
#include "wifi_hotspot.h" // Pour synchroniser la liste blanche avec le hotspot
#include "shared_data.h"

const char* deviceName = "ESP32_BLE_DEVICE";
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pNotifyCharacteristic = nullptr;

// Liste des codes de sécurité autorisés
const std::vector<std::string> securityCodes = {
    "dqihsqlp", "wkdqlqhl", "nywaiwul", "npvnqjpn",
    "pevtmobv", "hnwfeblw", "swynfvrl", "akgmnvec",
    "fhzeamrx", "ozxgzixh"
};

// Vérifie si un code de sécurité est valide
bool isCodeValid(const std::string& code) {
    return std::find(securityCodes.begin(), securityCodes.end(), code) != securityCodes.end();
}

// Vérifie si une adresse MAC est déjà autorisée
bool isMACAuthorized(const std::string& mac) {
    return std::find(whitelistMAC.begin(), whitelistMAC.end(), mac) != whitelistMAC.end();
}

// Supprime les espaces inutiles d'une chaîne
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos) return "";
    return str.substr(first, (last - first + 1));
}

// Vérifie si une MAC est valide (format XX:XX:XX:XX:XX:XX)
bool isValidMAC(const std::string& mac) {
    if (mac.length() != 17) return false;
    for (size_t i = 0; i < mac.length(); i++) {
        if (i % 3 == 2) {
            if (mac[i] != ':') return false;
        } else {
            if (!isxdigit(mac[i])) return false;
        }
    }
    return true;
}

// Ajoute une MAC autorisée si le code est valide
void authorizeMAC(const std::string& macAddress, const std::string& securityCode) {
    std::string trimmedMAC = trim(macAddress);
    std::string trimmedCode = trim(securityCode);
    std::string response;

    if (!isValidMAC(trimmedMAC)) {
        response = "ERROR: Invalid MAC format";
    } else if (!isCodeValid(trimmedCode)) {
        response = "ERROR: Invalid security code";
    } else if (isMACAuthorized(trimmedMAC)) {
        response = "INFO: MAC already authorized";
    } else {
        whitelistMAC.push_back(trimmedMAC);
        saveWhitelist();
        response = "SUCCESS: MAC " + trimmedMAC + " authorized";
    }

    Serial.println(response.c_str());  // ✅ Correction ici !

    if (pNotifyCharacteristic != nullptr) {
        pNotifyCharacteristic->setValue(response);
        pNotifyCharacteristic->notify();
        delay(100);  // Petit délai pour assurer l'envoi
    }
}


// Callbacks pour la caractéristique BLE
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        Serial.printf("BLE Received: %s\n", value.c_str());

        if (value.empty()) {
            Serial.println("ERROR: Empty BLE message received.");
            if (pNotifyCharacteristic != nullptr) {
                pNotifyCharacteristic->setValue("ERROR: Empty message");
                pNotifyCharacteristic->notify();
            }
            return;
        }

        // Vérification du format "MAC:XX:XX:XX:XX:XX:XX, CODE:xxxxxxx"
        size_t macPos = value.find("MAC:");
        size_t codePos = value.find("CODE:");

        if (macPos == std::string::npos || codePos == std::string::npos) {
            Serial.println("ERROR: Invalid format. Use 'MAC:<address>, CODE:<code>'");
            if (pNotifyCharacteristic != nullptr) {
                pNotifyCharacteristic->setValue("ERROR: Invalid format");
                pNotifyCharacteristic->notify();
            }
            return;
        }

        std::string macAddress = value.substr(macPos + 4, codePos - macPos - 5);
        std::string securityCode = value.substr(codePos + 5);

        // Nettoyage des espaces
        macAddress = trim(macAddress);
        securityCode = trim(securityCode);

        if (macAddress.empty() || securityCode.empty()) {
            Serial.println("ERROR: Empty MAC or CODE field.");
            if (pNotifyCharacteristic != nullptr) {
                pNotifyCharacteristic->setValue("ERROR: Empty MAC or CODE");
                pNotifyCharacteristic->notify();
            }
            return;
        }

        authorizeMAC(macAddress, securityCode);
    }
};

// Initialise le serveur BLE
void initializeBLE() {
    Serial.println("Starting BLE...");

    NimBLEDevice::init(deviceName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    Serial.println("BLE Device initialized");
    
    // Display BLE MAC address
    Serial.print("BLE MAC Address: ");
    Serial.println(NimBLEDevice::getAddress().toString().c_str());

    pServer = NimBLEDevice::createServer();
    Serial.println("BLE Server created");

    NimBLEService* pService = pServer->createService("12345678-1234-5678-1234-56789ABCDEF0");

    NimBLECharacteristic* pWriteCharacteristic = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789ABCDEF1",
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pWriteCharacteristic->setCallbacks(new CharacteristicCallbacks());
    Serial.println("Write characteristic created");

    pNotifyCharacteristic = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789ABCDEF2",
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
    );
    Serial.println("Notify characteristic created");

    pService->start();
    Serial.println("Service started");

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
    Serial.println("BLE advertising started");
}
