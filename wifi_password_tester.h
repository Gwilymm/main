#ifndef WIFI_PASSWORD_TESTER_H
#define WIFI_PASSWORD_TESTER_H

#include <Arduino.h>
#include <vector>

void initializePasswordTester();
void scanNetworks();
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
