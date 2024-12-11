#include "http_server.h"
#include <WiFi.h>
#include <map>  // Inclure <map> pour std::map

// Initialiser le serveur HTTP
WebServer server(80);

// Tableau des scores dynamique pour les connexions
std::map<String, ConnectionInfo> connectionScores;

// Fonction pour générer la page HTML du tableau de scores
String generateScorePage() {
    String page = "<html><body><h2>Tableau des Scores</h2><table border='1'><tr><th>Adresse MAC</th><th>Tentatives</th><th>Réussi</th></tr>";
    for (const auto& entry : connectionScores) {
        page += "<tr><td>" + entry.first + "</td><td>" + String(entry.second.attempts) + "</td><td>" + (entry.second.success ? "Oui" : "Non") + "</td></tr>";
    }
    page += "</table></body></html>";
    return page;
}

// Fonction de gestion des requêtes HTTP pour afficher le tableau de scores
// HTML response with no-cache headers and auto-refresh
void handleRoot() {
    String page = "<html><head>"
                  "<meta charset='UTF-8'>"
                  "<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'>"
                  "<meta http-equiv='Pragma' content='no-cache'>"
                  "<meta http-equiv='Expires' content='0'>"
                  "<meta http-equiv='refresh' content='5'>"  // Refresh page every 5 seconds
                  "</head><body><h2>Tableau des Scores</h2><table border='1'><tr><th>Adresse MAC</th><th>Tentatives</th><th>Réussi</th></tr>";

    for (const auto& entry : connectionScores) {
        page += "<tr><td>" + entry.first + "</td><td>" + String(entry.second.attempts) + "</td><td>" + (entry.second.success ? "Oui" : "Non") + "</td></tr>";
    }

    page += "</table></body></html>";

    server.send(200, "text/html; charset=UTF-8", page);
}


void handleTest() {
    server.send(200, "text/plain", "Test page is working!");
}

void initializeHttpServer() {
    server.on("/", handleRoot);
    server.on("/test", handleTest);  // Route de test
    server.begin();
    Serial.println("HTTP server started");
}


// Mettre à jour le tableau des scores pour chaque tentative de connexion
void updateConnectionScore(const String& macAddress, bool success) {
    if (connectionScores.find(macAddress) == connectionScores.end()) {
        connectionScores[macAddress] = {0, false};
    }
    connectionScores[macAddress].attempts++;
    connectionScores[macAddress].success = success;
}