#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <string>

void initializeBLE();
bool isMACAuthorized(const std::string& mac); // Vérifie si une MAC est autorisée

#endif // BLE_SERVER_H
