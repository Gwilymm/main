// ble_server.h
#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <vector>
#include <string>

extern std::vector<std::string> whitelistMAC;

void initializeBLE();
bool checkValidationReceived();
void authorizeMAC(const std::string& macAddress, const std::string& securityCode);

#endif
