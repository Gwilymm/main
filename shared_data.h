#pragma once
#include <vector>
#include <set>
#include <map>
#include <Arduino.h>

// Shared data declarations
extern std::vector<std::string> whitelistMAC;
extern std::set<String> loggedUnauthorizedMACs;
extern std::map<String, unsigned long> lastAttemptTime;
extern const unsigned long COOLDOWN_PERIOD;
