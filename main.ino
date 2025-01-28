#include <Arduino.h>
#include "tft_display.h"
#include "wifi_hotspot.h"
#include "ble_server.h"
#include "wifi_password_tester.h"

TaskHandle_t testPasswordsTaskHandle = NULL;
TaskHandle_t handleWiFiConnectionsTaskHandle = NULL;

void testPasswordsTask(void* parameter) {
    const char** ssidList = getFoundSSIDs();
    int ssidCount = getFoundSSIDCount();

    // Test des mots de passe pour tous les SSIDs trouvés
    testPasswords(ssidList, ssidCount);  // Remove the third argument

    Serial.println("Password testing completed.");
    vTaskDelete(NULL);  // Supprime la tâche une fois terminée
}

void handleWiFiConnectionsTask(void* parameter) {
    while (true) {
        handleWiFiConnections();  // Gérer les connexions Wi-Fi
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Attente non bloquante
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);  // Mode station Wi-Fi pour scanner et tester

    initializeTFT();                        // Initialiser l'écran TFT
    displayQRCode("https://gwilymm.github.io/espHack-security");  // Affiche un QR code

    initializeWiFiHotspot();                // Démarrer le hotspot Wi-Fi
    initializeBLE();                        // Initialiser le BLE pour validation

                // Initialiser le module de bruteforce
    scanNetworks();                         // Scanner et répertorier les réseaux disponibles

    // Créer des tâches FreeRTOS
    xTaskCreatePinnedToCore(
        testPasswordsTask,                  // Fonction de bruteforce
        "TestPasswordsTask",                // Nom de la tâche
        4096,                               // Taille de la pile
        NULL,                               // Paramètres passés à la tâche
        1,                                  // Priorité
        &testPasswordsTaskHandle,           // Handle pour gérer la tâche
        1                                   // Exécuter sur le second cœur
    );

    xTaskCreatePinnedToCore(
        handleWiFiConnectionsTask,          // Fonction de gestion des connexions
        "HandleWiFiConnectionsTask",        // Nom de la tâche
        4096,                               // Taille de la pile
        NULL,                               // Paramètres passés à la tâche
        1,                                  // Priorité
        &handleWiFiConnectionsTaskHandle,   // Handle pour gérer la tâche
        0                                   // Exécuter sur le premier cœur
    );

    Serial.println("Setup completed");
}

void loop() {
    // Rien ici, tout est géré via les tâches FreeRTOS
}
