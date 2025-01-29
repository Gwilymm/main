#include "wifi_hotspot.h"
#include "shared_data.h"
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <Arduino.h>

// Define the shared variables in only one source file
std::vector<std::string> whitelistMAC;
std::set<String> loggedUnauthorizedMACs;
std::map<String, unsigned long> lastAttemptTime;
std::set<String> loggedUnauthorizedMACs;  // Set to track logged unauthorized MAC addresses

const unsigned long COOLDOWN_PERIOD = 1000; // 1 second cooldown period

// Configuration du hotspot
const char* ssid_ap = "wifi_m2dfs_marin";
String chosenPassword;
const std::vector<String> passwords = {
"kxzvaajn", "nulebozl", "sylmukyv", "fzxhzodw", "mnhfwylw",
"xhnvtwdd", "jrulvtsp", "ochdamfa", "plaluafr", "ovnkkilc",
"oizuxgas", "tjrdbrav", "pklzocye", "vtapalxp", "rtagjdnv",
"frvuyeoe", "xghgxava", "rgolvnna", "yfxeqlny", "aewazmzb",
"shsiejiu", "iuvmohlb", "cjvxdfsl", "qpikserq", "utacmzqp",
"ooxevhdn", "sxthtwxj", "llrwdkcx", "ddudifar", "zphrewdm",
"mpdkrxev", "updhshlc", "tllzdwkl", "riyypzdz", "oemqckth",
"tffobpzd", "gtffxvfi", "txjtdwqy", "jfiqxhca", "sgypghxn",
"juixjinr", "aowreqdu", "swjeitqz", "igjautba", "aiyqeglh",
"oswyyore", "xptdcttd", "pcprabke", "kmqzndiw", "dpqzamtm",
"dyzcslls", "navqcfvd", "anowzypk", "suahetlj", "yfhccclh",
"qbmdivqr", "qblucjky", "dguzkxel", "ylpyzgjf", "kbfcdujc",
"ctuqkrdr", "ajevmebu", "skmcsqfz", "ffzlwiio", "aftglkvf",
"xxdgtzlw", "cvlfsunn", "bhdvmvku", "nfqzutau", "ggxbhrho",
"qyfalgmh", "tcxkbhrn", "qpogobwf", "cllvemtd", "vthbmclx",
"mygnkqqa", "whfrjrlf", "gwfeqkgf", "fkvgcrjw", "lezddzaj",
"xvczgqaa", "iekrpdyj", "ebkftonw", "eoudylkl", "ebxoxxfm",
"janmvljn", "xfudqipe", "yuyfhefw", "irxkfjbq", "kwdvygqs",
"yvhsdunb", "qhqzskuo", "nkcuqjjx", "treyrebr", "iubgkumg",
"iqeblqrp", "awllfhdq", "ywbvnsvp", "eqivzdwz", "feskhshm",
"fytbaolk", "lnqguosc", "mplxzpwm", "vsedeanw", "rvcnmtqq",
"fsptuhkw", "pmjuvole", "fxwnbwxl", "rnkilsjh", "uqpkhdhl",
"pqzxizwu", "lfgpxewt", "wbsvxglw", "adkfvisy", "moslxmvo",
"caiongpk", "mdfldeyy", "fsacirgb", "seosmnio", "zjafdgsg",
"xjhbsqbr", "emjtockn", "ymajxpiu", "nijknifq", "hbkawhxp",
"dxpygtws", "pkkkfsqc", "yfgydefu", "ocnbsmfu", "hqxndvfm",
"mpzxmzmc", "ynbxnvcb", "yvmmciwz", "nycmnnfk", "foitwmhx",
"wslaloll", "obsotlwf", "qcejxtcc", "jytvottv", "yhgoeojj",
"nqvxubel", "zekupkif", "iuxizxgu", "ksfdaahq", "rfdyxagk",
"aodiivkl", "nbomkfft", "gzjubrrt", "xkzfllsy", "zwnghvsp",
"lbdnrijp", "tztubmsb", "tmnfepsp", "dvhmvvak", "qldcakpn",
"gaykchmm", "viulztoy", "rgppnxmt", "gxhxirmi", "nkrhfpvl",
"cnuvswpy", "qitnplny", "wrjcumvb", "fmksndas", "cwotawuz",
"lzkfjavt", "deshqjlv", "hfcrpmqj", "stgtdpmk", "nealxloj",
"pqwygeet", "uuvqbkxw", "gyxuyioo", "rqkuzleq", "czuaaire",
"pyxcaiqc", "exxysayd", "jajnriij", "nchoqudf", "uzexsnfm",
"jajuagec", "rnutbptb", "ogvbnogl", "emwuzqan", "kcgzpxfo",
"fhgjzrxe", "nrncgdsx", "cywwydif", "lyuzcaqv", "iwzzufyn",
"qbfjmlfi", "tdnsshfc", "kssrcayx", "ifykxdys", "bazausxn",
"wahbnioc", "jvnqahyb", "jngbwrke", "nobrqqda", "qjoehftw"
};

// Charger un mot de passe aléatoire
String loadRandomPassword() {
    if (passwords.size() > 0) {
        randomSeed(esp_random()); // Initialiser le générateur aléatoire
        int index = random(0, passwords.size());
        return passwords[index];
    }
    return "defaultpass"; // Retourne un mot de passe par défaut
}


// Initialiser le point d'accès WiFi avec un mot de passe aléatoire
void initializeWiFiHotspot() {
    SPIFFS.begin(true); // Initialiser SPIFFS pour la persistance
    loadWhitelist(); // Charger la liste blanche au démarrage

    chosenPassword = loadRandomPassword();
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid_ap, chosenPassword.c_str());
    IPAddress IP_AP = WiFi.softAPIP();

    Serial.println("WiFi Hotspot Initialized");
    Serial.printf("SSID: %s\n", ssid_ap);
    Serial.printf("Password: %s\n", chosenPassword.c_str());
    Serial.printf("AP IP Address: %s\n", IP_AP.toString().c_str());
}

// Envoyer une trame de désauthentification
void sendDeauthFrame(uint8_t* mac) {
    uint8_t YOUR_AP_MAC[6];
    WiFi.softAPmacAddress().toCharArray((char*)YOUR_AP_MAC, 6);

    uint8_t deauth_frame[26] = {
        0xC0, 0x00, // Frame Control
        0x00, 0x00, // Duration
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], // Destination MAC
        YOUR_AP_MAC[0], YOUR_AP_MAC[1], YOUR_AP_MAC[2], YOUR_AP_MAC[3], YOUR_AP_MAC[4], YOUR_AP_MAC[5], // Source MAC
        YOUR_AP_MAC[0], YOUR_AP_MAC[1], YOUR_AP_MAC[2], YOUR_AP_MAC[3], YOUR_AP_MAC[4], YOUR_AP_MAC[5], // BSSID
        0x00, 0x00, // Sequence Control
        0x01, 0x00  // Reason Code
    };
    if (!mac) {
        Serial.println("Erreur : Adresse MAC client invalide.");
        return;
    }

    if (sizeof(deauth_frame) != 26) {
        Serial.println("Erreur : Trame mal formée.");
        return;
    }

    // Initialize AP
    WiFi.softAP(ssid_ap, chosenPassword.c_str());
    IPAddress IP_AP = WiFi.softAPIP();
    Serial.println("WiFi Hotspot Initialized");
    
    Serial.print("SSID (AP): ");
    Serial.println(ssid_ap);
    Serial.print("Password (AP): ");
    Serial.println(chosenPassword);
    Serial.print("AP IP Address: ");
    Serial.println(IP_AP);
    esp_err_t err = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame, sizeof(deauth_frame), false);
        if (err == ESP_ERR_WIFI_NOT_INIT) {
            Serial.println("Erreur : Wi-Fi non initialisé.");
        } else if (err == ESP_ERR_INVALID_ARG) {
            Serial.println("Erreur : Arguments invalides pour esp_wifi_80211_tx.");
        } else if (err == ESP_ERR_WIFI_IF) {
            Serial.println("Erreur : Interface Wi-Fi incorrecte.");
        } else if (err == ESP_OK) {
            Serial.println("Trame de désauthentification envoyée avec succès.");
        } else {
            Serial.printf("Erreur inattendue : %s\n", esp_err_to_name(err));
        }

}

// Gérer les connexions WiFi
void handleWiFiConnections() {
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);

    unsigned long currentMillis = millis();
    for (int i = 0; i < stationList.num; i++) {
        wifi_sta_info_t station = stationList.sta[i];
        String mac = String(station.mac[0], HEX) + ":" + 
                     String(station.mac[1], HEX) + ":" + 
                     String(station.mac[2], HEX) + ":" + 
                     String(station.mac[3], HEX) + ":" + 
                     String(station.mac[4], HEX) + ":" + 
                     String(station.mac[5], HEX);
        mac.toUpperCase();

        if (isMACAuthorized(mac)) {
            Serial.printf("Appareil autorisé : %s\n", mac.c_str());
        } else {
            if (currentMillis - lastAttemptTime[mac] > COOLDOWN_PERIOD) {
                lastAttemptTime[mac] = currentMillis;
                Serial.printf("Appareil non autorisé : %s\n", mac.c_str());
                sendDeauthFrame(station.mac);
            }
        }
    }
}