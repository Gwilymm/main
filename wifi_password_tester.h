#ifndef WIFI_PASSWORD_TESTER_H
#define WIFI_PASSWORD_TESTER_H

#include <vector>
#include <Arduino.h>


void initializePasswordTester();
void scanNetworks();
bool testPassword(const char* ssid, const String& password);
void testPasswords(const char* ssidList[], int ssidCount);
void testAllSSIDs();
const char** getFoundSSIDs();
int getFoundSSIDCount();
void updateConnectionScore(const String& ssid, bool success, bool unauthorized);  // Declare updateConnectionScore
void testWiFiConnection(void* parameter);  // Declare the FreeRTOS task function
void initializePromiscuousMode();  // Declare the function to initialize promiscuous mode
void processDataFrame(const uint8_t* data, int len);
String extractMAC(const uint8_t* data, int offset);

extern bool testInProgress;  // Declare global variable for test status
extern bool connectionResult;  // Declare global variable for connection result

#endif // WIFI_PASSWORD_TESTER_H
