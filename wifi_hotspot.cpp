#include "wifi_hotspot.h"
#include "ble_server.h"
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include "tft_display.h"
#include <vector>
#include <random>
#include <Arduino.h>
#include <map>
#include <chrono>
#include <set>

std::vector<std::string> displayedMACs;
std::map<String, unsigned long> lastAttemptTime;
std::set<String> loggedUnauthorizedMACs;  // Set to track logged unauthorized MAC addresses

const unsigned long COOLDOWN_PERIOD = 1000; // 1 second cooldown period

// Shared data
std::vector<String> targetSSIDs;
const std::vector<String> passwords = {
  "kxzvaajn","nulebozl","sylmukyv","fzxhzodw","mnhfwylw","xhnvtwdd","jrulvtsp","ochdamfa","plaluafr","ovnkkilc","oizuxgas","tjrdbrav","pklzocye","vtapalxp","rtagjdnv","frvuyeoe","xghgxava","rgolvnna","yfxeqlny","aewazmzb","shsiejiu","iuvmohlb","cjvxdfsl","qpikserq","utacmzqp","ooxevhdn","sxthtwxj","llrwdkcx","ddudifar","zphrewdm","mpdkrxev","updhshlc","tllzdwkl","riyypzdz","oemqckth","tffobpzd","gtffxvfi","txjtdwqy","jfiqxhca","sgypghxn","juixjinr","aowreqdu","swjeitqz","igjautba","aiyqeglh","oswyyore","xptdcttd","pcprabke","kmqzndiw","dpqzamtm","dyzcslls","navqcfvd","anowzypk","suahetlj","yfhccclh","qbmdivqr","qblucjky","dguzkxel","ylpyzgjf","kbfcdujc","ctuqkrdr","ajevmebu","skmcsqfz","ffzlwiio","aftglkvf","xxdgtzlw","cvlfsunn","bhdvmvku","nfqzutau","ggxbhrho","qyfalgmh","tcxkbhrn","qpogobwf","cllvemtd","vthbmclx","mygnkqqa","whfrjrlf","gwfeqkgf","fkvgcrjw","lezddzaj","xvczgqaa","iekrpdyj","ebkftonw","eoudylkl","ebxoxxfm","janmvljn","xfudqipe","yuyfhefw","irxkfjbq","kwdvygqs","yvhsdunb","qhqzskuo","nkcuqjjx","treyrebr","iubgkumg","iqeblqrp","awllfhdq","ywbvnsvp","eqivzdwz","feskhshm","fytbaolk","lnqguosc","mplxzpwm","vsedeanw","rvcnmtqq","fsptuhkw","pmjuvole","fxwnbwxl","rnkilsjh","uqpkhdhl","pqzxizwu","lfgpxewt","wbsvxglw","adkfvisy","moslxmvo","caiongpk","mdfldeyy","fsacirgb","seosmnio","zjafdgsg","xjhbsqbr","emjtockn","ymajxpiu","nijknifq","hbkawhxp","dxpygtws","pkkkfsqc","yfgydefu","ocnbsmfu","hqxndvfm","mpzxmzmc","ynbxnvcb","yvmmciwz","nycmnnfk","foitwmhx","wslaloll","obsotlwf","qcejxtcc","jytvottv","yhgoeojj","nqvxubel","zekupkif","iuxizxgu","ksfdaahq","rfdyxagk","aodiivkl","nbomkfft","gzjubrrt","xkzfllsy","zwnghvsp","lbdnrijp","tztubmsb","tmnfepsp","dvhmvvak","qldcakpn","gaykchmm","viulztoy","rgppnxmt","gxhxirmi","nkrhfpvl","cnuvswpy","qitnplny","wrjcumvb","fmksndas","cwotawuz","lzkfjavt","deshqjlv","hfcrpmqj","stgtdpmk","nealxloj","pqwygeet","uuvqbkxw","gyxuyioo","rqkuzleq","czuaaire","pyxcaiqc","exxysayd","jajnriij","nchoqudf","uzexsnfm","jajuagec","rnutbptb","ogvbnogl","emwuzqan","kcgzpxfo","fhgjzrxe","nrncgdsx","cywwydif","lyuzcaqv","iwzzufyn","qbfjmlfi","tdnsshfc","kssrcayx","ifykxdys","bazausxn","wahbnioc","jvnqahyb","jngbwrke","nobrqqda","qjoehftw","kfiervay","frfdnemp","uogvaxoy","mrxsoriz","ripscnac","lcybccwa","grvwfjre","vfpovxfb","drxegbnw","uqgtcomf","vffsurhg","rkgppqld","eihmjela","wmjugifs","wzphzrqp","ruoqvvsz","wkqyiobj","qqsfalig","jnqbfzit","foypedma","ilitsfdb","ydgyiufn","kqksvrcb","vlkefimy","xekeszqb","laqwcarr","tczmmbmd","xcpqbkle","lsrzadtr","kranxvtl","wpnzmldn","vjzehoza","degnhpod","jdodpgih","wawushuj","pkiffncv","gujtopcc","jcfpuuhz","bkgvwxqm","egxpnhwe","aigxcimr","ktcjtsde","exwkxrce","zgwimqaq","xjoqqekr","aadjarnf","ijsdfwgp","pynhcmql","lpqitgat","tnqxtyem","vfdepkeo","yqlsegfv","mbowelxc","ugaduxtj","rymrjtuc","ytwybsjt","cnzvqkuq","kyrndbcr","qaikjtff","pjdgxcfc","uhqesvtg","moizoowx","mklhyzrl","xyffokve","iuyzohge","czaxxdxi","saplbnbf","dowulvck","tfemjvjk","ymttuiad","sxrdfnaa","alfzxmva","dgpavcqy","hvnfmqui","ybxyecfn","vovicbcs","gcaugaji","zbfnipmu","radcqnmq","wlaooask","drrjbkbc","cxrgwksp","tlvydckx","garuthzm","nikhlzwo","epntjhdr","srxruwie","ypsbwgpi","eotqvuco","pvvilcen","plrgilnr","pcoddugn","tdvjzuky","wbwfxbbn","ozkdiica","pknslulj","hsbywqcf","fbasjjzt","gkjdboor","konjbhou","hwoineyg","eoqmjdno","bslvnkfg","gbkkfjcw","ewgtzwix","cascwmws","lyrkwzzf","ogavhxqd","kulsyieh","tfccxbfa","cuxvfddt","azkvujdj","ksfvhsii","wxdxwbvy","nuuanlbr","jtbwwmld","ihfpxekr","mnaezogi","vuppqwvj","aqodlnnt","dgqmspec","peccelzo","bmxmvdnk","evfoxuut","msbhavqg","upbtline","djloihdn","gqnzmvnv","kwatsoka","jkwjbozo","trwajtyt","jdgkawpa","mwbaxfop","mvnlluac","nxmzbtau","ymuyagbq","kauwaawp","nbyuqarg","hxqxhdul","fmtejpda","pqjuytuj","ahoxacvy","qlsewxaf","rdxevtnq","iakmdjnl","xrmagiaz","hnkfeclh","czunysfs","fudgrhfx","xlkfvorn","rmbzvczf","tfizbxkr","ecaagcoc","dtopbstn","qsbixyif","hvylbpxw","dmtfeceh","ipglledi","wukxltvl","macakpog","nqerrybr","uqarfzeg","bxfcogys","akjxypym","onlrjaee","hzdzlwxu","qoforywv","eliclhao","lubhscai","nqihmmvq","gncmlyql","zqjhanzj","lchwfmyz","wcyhijrc","titkjzfl","omzhbawx","wjovimvl","ygogrlxi","poxquwbr","ivwddsne","giocvkmz","mckqexaq","sjihxtdo","hqtammmu","ccjtugqr","gkbhyxqc","wswhmjxi","eavbbnnd","slulgloi","foillccp","hlcemxhx","pkrdwvzy","vilwqsnl","yhilpqtw","vsmwzxxi","hpwsofok","rbdpjukh","gignctwt","ktvmbnuo","oehjktzb","oagkhllv","jwemvifg","kafnjvzl","djqfkckt","ydjjsoop","jjmkbvuj","zpdefbxp","fftycivb","kbhdxnea","rgkotlbq","hogjwlxo","okqdnzxg","kzzuqrdp","vqvjiwkr","wmbalnvy","lwaffsni","pjbacohx","wbodbadu","ejztinqe","yhhnuisj","vwpnrxjn","vhwylhra","sxtshznp","qhvjxaom","pfsnvhmf","fvpwayeg","hzdooswm","tackidly","svkbdjfz","ffntpqpk","tqrccxyq","volpqmic","dfzhvwar","zehfllui","bsfhhddr","nktmelwu","cctduqoz","gzgcuxcw","dqhlgfhn","dmapgbvv","ofskwsqe","kgaxogdr","spwpsbhf","bopsadyf","abpaaigj","jgxstcil","hsfdcfxv","ymasoylx","ahbtwatm","dlijcblt","sdhlarff","spjypnwo","buljqpcj","dotzuuyv","pheevrpe","mahurrzq","wpqafeld","venyuuyg","tnqbubhu","rltnkhtn","cuklejud","bvwsltfo","yviapbga","vutjxfqz","fnkjmkoh","tglljmrx","dvpdwwds","izdrtolt","rrxaprdw","sztcxxex","uddtvznv","eckuuaao","vmgsnunc","nxtmmzkn","kgvyqmxq","cnczmbef","lurcwgkk","nmlbygfm","wfcningm","jebhscka","ytkuhsoc","zufpxqmq","nrpnnfqa","vwiyrxpu","yjfudkex","rmtskwpm","eppkpjzs","wkwkynkd","qgbpfmzu","vcybncik","kdqqkksl","ebxwfwdy","pcnlnrlc","wxemtreg","pogmuheh","mdgzdjih","xrvwzkkn","liusnvca","taquiwtb","mdecmjge","fussbiag","ddfaqnyc","xefnqfmv","hawzmtzv","rgscnzcu","vjjmxctn","deegjkqm","vreoilwx","wruesesw","mryacxfh","wsffrihm","tkohpnzm","yggjipux","qvwlpjiu","xngfskii","pikiitmm","ahnomypa","bcsahndu","iniovsqv","qvayqwax","zhjmzlte","bpowqubr","pwcxbnyz","ihtsiwel","oadocela","pxrsyhru","wkwlkikn","hnxnuhfi","tbxrluzr","fdutyhxd","lcqyvunh","kifruxmd","agmimjwa","bhtkxvtc","piochrkv","rxssmvsq","yqddvdlf","tvpsgghl","osxnojfy","tgtcbjlq","blqacehn","vywukhpq","zwvqndzv","ebfrjjlj","aknbkjvy","snwvxoaw","oilegcik","qbgttlyx","rmysakvw","bdkvjyfk","rfrsukxo","adtbmqxw","erwjapot","drwkfmxs","kcsqdwcz","ijkxtjql","lycbrnvw","cbjhkvrv","zzqnrlkr","xlaefzkk","lzhsxveu","wgmdiezu","bcknfyke","mcwoaewz","fpvimxem","kqjouhrj","lpusuvgq","fstoijge","ypoqbmut","arsusbza","ouektgkr","lvlkzmzk","omgwssle","bjxzqjbn","hvmhtdfk","mnwtyjcb","bfrndlkt","qktutswj","lsrtvtdi","rzwxwjic","nmhmgrte","gpcpaapp","fgczzugh","aspkmvfs","hzihhmsg","kvukwynb","ljxjjfec","btajnnss","hlhzuybi","bttbeiyp","iiesorra","figvinjs","gurxckig","aawasxbe","smbxeoak","ziofnxkc","xowcptyv","xzjldfat","nlbypshy","jptvbmiq"
};

// Nom du réseau WiFi et mot de passe
const char* ssid_ap = "WIFI_M2DFS_MARIN";
String chosenPassword;  // Mot de passe choisi pour le hotspot WiFi

// Charger un mot de passe aléatoire depuis SPIFFS
String loadRandomPassword() {
    if (passwords.size() > 0) {
        randomSeed(esp_random());  // Initialiser le générateur de nombres aléatoires
        int index = random(0, passwords.size());
        return passwords[index];
    } else {
        Serial.println("Aucun mot de passe disponible dans la liste");
        return "";
    }
}


// Initialiser le point d'accès WiFi avec un mot de passe aléatoire
void initializeWiFiHotspot() {
    chosenPassword = loadRandomPassword();
    if (chosenPassword == "") {
        Serial.println("Aucun mot de passe disponible, point d'accès non configuré");
        return;
    }

    WiFi.mode(WIFI_AP_STA);  // Initialize WiFi in both AP and STA modes

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

    // Initialize STA
    // Explicitly call scanNetworks with an empty string
}

// Gestion des connexions WiFi
 // Ensemble des adresses MAC déjà affichées

// Replace with your AP's MAC address
uint8_t YOUR_AP_MAC[6] = {0x3C, 0xE9, 0x0E, 0x6C, 0xC5, 0x05};

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

        // Convertir l'adresse MAC en std::string pour l'utiliser avec std::vector
        std::string macStd = std::string(mac.c_str());

        // Vérifier si l'adresse MAC est dans la liste blanche
        bool isAuthorized = std::find(whitelistMAC.begin(), whitelistMAC.end(), macStd) != whitelistMAC.end();

        // Check if the MAC address is within the cooldown period
        if (lastAttemptTime.find(mac) == lastAttemptTime.end() || (currentMillis - lastAttemptTime[mac] > COOLDOWN_PERIOD)) {
            lastAttemptTime[mac] = currentMillis;

            if (isAuthorized) {
                Serial.print("Appareil autorisé : ");
                Serial.println(mac);
                updateConnectionScore(mac, true, false); // Update the screen with the connection attempt
            } else {
                // Check if the MAC address has already been logged
                if (loggedUnauthorizedMACs.find(mac) == loggedUnauthorizedMACs.end()) {
                    Serial.print("Appareil non autorisé : ");
                    Serial.println(mac);
                    loggedUnauthorizedMACs.insert(mac);  // Add to logged set
                }

                // Déconnecter l'appareil non autorisé sans affecter le hotspot
                // Construct a deauth frame
                uint8_t deauth_frame[26] = {
                    0xC0, 0x00, // frame control: Deauth
                    0x00, 0x00, // duration
                    station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5], // destination (the station)
                    YOUR_AP_MAC[0], YOUR_AP_MAC[1], YOUR_AP_MAC[2], YOUR_AP_MAC[3], YOUR_AP_MAC[4], YOUR_AP_MAC[5], // source (your AP)
                    YOUR_AP_MAC[0], YOUR_AP_MAC[1], YOUR_AP_MAC[2], YOUR_AP_MAC[3], YOUR_AP_MAC[4], YOUR_AP_MAC[5], // BSSID (your AP)
                    0x00, 0x00, // sequence control
                    0x07, 0x00  // reason code: Class 3 frame received from nonassociated STA
                };

                esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame, sizeof(deauth_frame), false);
                updateConnectionScore(mac, false, true); // Update the screen with the connection attempt

                delay(100);  // Add a small delay to avoid overwhelming the WiFi driver
            }

            // Ajouter l'adresse MAC à la liste des adresses déjà affichées si elle n'y est pas déjà
            if (std::find(displayedMACs.begin(), displayedMACs.end(), macStd) == std::end(displayedMACs)) {
                displayedMACs.push_back(macStd);
            }

            // Update the MAC list for display
            updateMACList(mac, currentMillis);
        }
    }
}