#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <Arduino.h>
#include <WebServer.h>  // Utilisation de WebServer pour le serveur HTTP
#include <map>

// Structure pour stocker les informations de connexion
struct ConnectionInfo {
    int attempts;
    bool success;
};

// Déclaration du serveur HTTP pour l'accès global
extern WebServer server;

// Initialiser le serveur HTTP
void initializeHttpServer();

// Mettre à jour le tableau des scores
void updateConnectionScore(const String& macAddress, bool success);

#endif  // HTTP_SERVER_H
