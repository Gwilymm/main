# Hack the wifi

## Concept

L'idée pour ce challenge était de proposer une double authentification en utilisant le BLE et les capacité Wifi de l'esp32.

## Composant Hotspot WiFi

### Vue d'ensemble

Le composant hotspot WiFi crée un point d'accès sécurisé intégrant un filtrage des adresses MAC et une déauthentification automatique des clients non autorisés. Il est conçu pour démontrer les concepts de sécurité et de contrôle d'accès réseau.

### Fonctionnalités principales

1. **Gestion dynamique des mots de passe**

    - Liste prédéfinie de 200 mots de passe possibles

    - Sélection aléatoire d'un mot de passe au démarrage

    - Persistance du mot de passe entre les redémarrages

    - Format : chaînes alphanumériques minuscules de 8 caractères

2. **Liste blanche des adresses MAC**

    - Stockage persistant dans SPIFFS

    - Fonctionnalités de chargement/enregistrement de la liste blanche

    - Vérification en temps réel des adresses MAC

    - Déauthentification automatique des appareils non autorisés

3. **Mesures de sécurité**

    - Délai de refroidissement entre les tentatives de connexion (1000 ms)

    - Journalisation des tentatives d'accès non autorisées

    - Implémentation d'une trame de déauthentification personnalisée

    - Système de vérification des adresses MAC

### Spécifications techniques

#### Configuration du point d'accès

```
SSID : "wifi_m2dfs_marin"
Mot de passe : Sélectionné aléatoirement dans la liste de mots de passe
Mode : WIFI_AP_STA (Mode simultané Point d'Accès et Station)
```

#### Structure de la trame de déauthentification

```
Contrôle de trame :     2 octets
Durée :                 2 octets
MAC de destination :    6 octets
MAC source :           6 octets
BSSID :                6 octets
Contrôle de séquence :  2 octets
Code de raison :       2 octets
Taille totale :        26 octets
```

### Fonctions principales

1. **initializeWiFiHotspot()**

    - Initialise SPIFFS

    - Charge la liste blanche des adresses MAC

    - Sélectionne un mot de passe aléatoire

    - Configure le mode AP

    - Démarre le point d'accès

2. **handleWiFiConnections()**

    - Surveille les clients connectés

    - Vérifie les adresses MAC

    - Applique les délais de refroidissement

    - Déclenche la déauthentification si nécessaire

3. **sendDeauthFrame()**

    - Crée une trame de déauthentification 802.11

    - Inclut des vérifications d'erreur

    - Implémente la structure correcte de la trame

    - Gère les erreurs de transmission

4. **Gestion de la liste blanche**

    - saveWhitelist() : Stocke les adresses dans SPIFFS

    - loadWhitelist() : Charge les adresses depuis SPIFFS

    - isMACAuthorized() : Vérifie l'autorisation

### Structures de données

#### Variables globales

```
whitelistMAC : Vecteur des adresses MAC autorisées
loggedUnauthorizedMACs : Ensemble des tentatives non autorisées
lastAttemptTime : Association des MAC aux dernières tentatives
COOLDOWN_PERIOD : 1000 ms entre chaque tentative
```

#### Liste de mots de passe

- 200 mots de passe pré-générés

- 8 caractères chacun

- Stockés en mémoire flash

### Gestion des erreurs

1. **Opérations SPIFFS**

    - Détection des erreurs d'ouverture/lecture/écriture

    - Mécanismes de récupération

    - Journalisation des erreurs

2. **Opérations WiFi**

    - Gestion des échecs de connexion

    - Détection des adresses MAC invalides

    - Erreurs liées aux trames de déauthentification

### Considérations de sécurité

1. **Contrôle d'accès**

    - Filtrage des adresses MAC

    - Déauthentification des clients non autorisés

    - Application des délais de refroidissement

2. **Protection des données**

    - Stockage sécurisé de la liste blanche

    - Sélection aléatoire des mots de passe

    - Limitation des tentatives de connexion

3. **Limitations connues**

    - Les adresses MAC peuvent être usurpées

    - Les trames de déauthentification ne sont pas chiffrées

    - Limitations des capacités AP de l'ESP32

### Exemple d'implémentation

```
// Initialisation et démarrage du hotspot
initializeWiFiHotspot();

// Boucle principale
void loop() {
    handleWiFiConnections();
    // Autres tâches...
}
```

### Débogage

1. **Sortie série**

    - Tentatives de connexion

    - Adresses MAC

    - État de l'autorisation

    - Événements de déauthentification

2. **Messages d'erreur**

    - Opérations SPIFFS

    - Initialisation WiFi

    - Transmission des trames

    - Vérification des MAC

### Améliorations futures

## Guide d'implémentation du serveur BLE ESP32

### Architecture du serveur BLE

#### Composants principaux

1. **Configuration du serveur**

    ```
    Nom de l'appareil : "ESP32_BLE_DEVICE"
    Niveau de puissance : ESP_PWR_LVL_P9
    UUID du service : "12345678-1234-5678-1234-56789ABCDEF0"
    Caractéristique d'écriture : "12345678-1234-5678-1234-56789ABCDEF1"
    Caractéristique de notification : "12345678-1234-5678-1234-56789ABCDEF2"
    ```

2. **Mise en œuvre de la sécurité**

    - Liste pré-définie de codes de sécurité

    - Liste blanche des adresses MAC

    - Système d'autorisation

    - Système de notification des réponses

### Protocole de communication

1. **Format des requêtes client**

    ```
    MAC:<adresse_mac_appareil>, CODE:<code_de_sécurité>
    ```

    Exemple : `MAC:AA:BB:CC:DD:EE:FF, CODE:dqihsqlp`

2. **Réponses du serveur**

    - Succès : "MAC AUTORISÉE"

    - Échec : "CODE INVALIDE"

    - Réponse transmise via la caractéristique de notification

### Détails de l'implémentation

1. **Séquence d'initialisation**

    - Impression de l'adresse MAC (STA et AP)

    - Initialisation de NimBLE

    - Création du serveur

    - Configuration des services

    - Création des caractéristiques

    - Configuration des annonces

    - Démarrage du service

2. **Validation des codes de sécurité**

    - Ensemble fixe de 10 codes pré-définis

    - Validation sensible à la casse

    - Pas de mécanisme d'expiration

    - Validation à usage unique

3. **Gestion des adresses MAC**

    - Liste blanche stockée en mémoire

    - Fonctionnalité de suppression des adresses MAC

    - Vérification des doublons

    - Comparaison insensible à la casse

### Limitations connues

1. **Contraintes matérielles**

    - Maximum de 9 connexions simultanées

    - Portée de diffusion limitée

    - Consommation énergétique élevée

2. **Limitations logicielles**

    - Aucune persistance après redémarrage

    - Liste de codes de sécurité fixe

    - Absence de chiffrement

3. **Considérations de sécurité**

    - Communication en texte clair

    - Possibilité d'usurpation des adresses MAC

    - Pas de gestion de session

## Documentation du Testeur de Mot de Passe WiFi

### Vue d’ensemble

Le module Testeur de Mot de Passe WiFi permet de scanner des réseaux WiFi spécifiques et de tester systématiquement des mots de passe contre eux. Il est conçu pour fonctionner avec les réseaux dont les SSID correspondent au modèle *_"wifi_m2dfs_"**.

### Fonctionnalités principales

- Recherche et enregistre jusqu’à **14 SSID cibles**
- Tentatives de connexion non bloquantes (délai d’attente de **750 ms**)
- Test séquentiel des mots de passe
- Suivi en **temps réel** de la progression
- Enregistrement des succès/échecs pour chaque SSID

----------

### Fonctions principales

#### `void initializePasswordTester()`

Initialise le module de test des mots de passe. Actuellement réservé pour de futurs besoins d’initialisation.

#### `void scanNetworks()`

Effectue un scan des réseaux WiFi :

- Recherche les réseaux correspondant au modèle __"wifi_m2dfs_"_*
- Stocke jusqu’à **MAX_SSIDS (50)** réseaux détectés
- Affiche les résultats du scan via **Serial**
- Met à jour le stockage interne des SSID

#### `const char** getFoundSSIDs()`

Retourne un tableau des **SSID détectés**.

- **Type de retour :** Tableau de chaînes de caractères C-style
- **Utilisation :** Accès aux SSID stockés

#### `int getFoundSSIDCount()`

Retourne le **nombre de SSID détectés**.

- **Type de retour :** Entier
- **Plage de valeurs :** 0 à **MAX_SSIDS (50)**

#### `bool isSSIDTarget(const String& ssid)`

Vérifie si un **SSID appartient à la liste cible**.

- **Paramètre :** SSID sous forme de chaîne (`String`)
- **Retourne :** `true` si le SSID est dans la liste cible

#### `void testPasswords(const char* ssidList[], int ssidCount, const std::vector<String>& passwordList)`

**Fonction principale de test des mots de passe.**

**Paramètres :**

- `ssidList` : Tableau des SSID à tester
- `ssidCount` : Nombre de SSID dans la liste
- `passwordList` : Vecteur des mots de passe à essayer

**Comportement :**

1. Teste chaque SSID **séquentiellement**
2. Tente tous les mots de passe sur le SSID en cours
3. Temps d’attente de **750 ms** par tentative
4. Affiche les résultats en cas de connexion réussie
5. Enregistre les résultats dans **Serial**

**Exemple de sortie :**

```
=========================
Test du SSID 1/3 : wifi_m2dfs_test
=========================
Tentative de mot de passe 1/200

SUCCÈS ! Connecté à wifi_m2dfs_test
Mot de passe trouvé : testpass123
Tentative : 1/200

```

----------

### Variables globales

- `testInProgress` : Indique si un **test est en cours**
- `connectionResult` : Stocke le **dernier résultat de connexion**
- `foundSSIDs` : Tableau des **SSID détectés**
- `foundSSIDCount` : Nombre total de **SSID découverts**
- `capturedSSIDs` : Vecteur des **SSID auxquels l’accès a réussi**
- `capturedPasswords` : Vecteur des **mots de passe valides**

### Constantes

```cpp
#define MAX_SSIDS 50  // Nombre maximum de SSID stockables

```

----------

### Dépendances

- `Arduino.h`
- `WiFi.h`
- `esp_wifi.h`
- `tft_display.h`
- `wifi_hotspot.h`

----------

### Gestion des erreurs

- Signale les **échecs de scan WiFi**
- Gère les **délai d’attente dépassés** (`timeout`)
- Prend en charge les **déconnexions**
- Vérifie le **nombre de SSID valides**

----------

### Considérations de performance

- Délai de **750 ms** par tentative de connexion
- Le test **séquentiel** peut être long sur plusieurs SSID
- La consommation **mémoire** augmente avec `MAX_SSIDS`

----------

### Notes d’intégration

- Nécessite un **sous-système WiFi initialisé**
- Dépend d’un **affichage TFT** pour afficher l’état
- Utilise `wifi_hotspot.h` pour récupérer la liste des mots de passe

----------

### Exemple d’utilisation

```cpp
// Initialisation
initializePasswordTester();

// Scan des réseaux WiFi
scanNetworks();

// Récupération des résultats
const char** ssids = getFoundSSIDs();
int count = getFoundSSIDCount();

// Test des mots de passe
std::vector<String> passwords = getPasswordList(); // Récupéré depuis wifi_hotspot
testPasswords(ssids, count, passwords);

```

## Documentation de l'Affichage TFT

### Vue d’ensemble

Le module d'affichage TFT gère l'interface visuelle de la plateforme de test de sécurité ESP32 en utilisant un écran **ILI9341**. Il fournit des **informations en temps réel** via des écrans rotatifs affichant des **QR codes, des statistiques de connexion et des listes d'adresses MAC**.

---

### Configuration Matérielle

```cpp
#define TFT_CS   15      // Broche Chip Select
#define TFT_DC   2       // Broche Data/Command
#define TFT_RST  -1      // Broche Reset (non utilisée)
#define TFT_MOSI 13      // Broche SPI MOSI
#define TFT_SCLK 14      // Broche SPI Clock
#define TFT_BL   27      // Broche de rétroéclairage

```

----------

### Fonctionnalités principales

- **Affichage automatique rotatif** (toutes les 10 secondes)
- **Génération de QR codes** pour les identifiants WiFi
- **Statistiques des connexions réussies**
- **Surveillance des adresses MAC**
- **Affichage paginé** pour gérer les grands ensembles de données
- **Mises à jour d’écran multi-threadées**

----------

### Fonctions principales

#### `void initializeTFT()`

Initialise l'écran TFT :

- Configure la **communication SPI**
- Définit la **rotation de l'affichage**
- Initialise le **rétroéclairage**
- Crée une **tâche de mise à jour d’écran**

#### `void screenTask(void *pvParameters)`

Tâche en arrière-plan qui gère la rotation de l'affichage :

- **Exécutée sur un thread dédié**
- Met à jour l'écran **toutes les 10 secondes**
- Gère les **transitions entre écrans**

#### `void alternateScreens()`

Gère la rotation entre :

- **Affichage du QR code**
- **Statistiques de succès**
- **Liste des adresses MAC**
- Efface l'écran **entre chaque transition**

#### `void displayQRCode(const char* text)`

Génère et affiche un **QR code** contenant les identifiants WiFi :

- Utilise un **niveau de correction d'erreur 3**
- Redimensionne le QR code pour une **visibilité optimale**
- Inclut **l’SSID et le mot de passe**

#### `void displayMACList()`

Affiche une **liste paginée des adresses MAC** :

- **5 entrées par page**
- Changement de page **toutes les 10 secondes**
- Affiche les **tentatives de connexion et leur statut**

#### `void displaySuccessConnections()`

Affiche les **statistiques des connexions réussies** :

- Liste les **SSID ayant réussi**
- Affiche le **nombre de tentatives**
- Mise à jour en **temps réel**

----------

### Structures de données

```cpp
struct ConnectionInfo {
    int attempts;
    bool success;
};

std::map<String, unsigned long> macList;
std::map<String, ConnectionInfo> connectionScores;

```

----------

### Types d'écrans

1. **Écran QR Code**

    - **Identifiants réseau WiFi**
    - Affichage du **SSID**
    - **QR code généré automatiquement**
2. **Écran de Succès**

    - **Liste des connexions réussies**
    - Nombre de **tentatives**
    - Indicateurs de **succès**
3. **Écran Liste MAC**

    - **Adresses MAC des appareils connectés**
    - Statistiques de connexion
    - **Affichage paginé**

----------

### Considérations de performance

- **Mise à jour de l’écran** exécutée dans une tâche dédiée
- Intervalle de **rotation de 10 secondes**
- **Buffer mémoire optimisé** pour l'affichage
- **Génération efficace des QR codes**

----------

### Points d’intégration

- Interface avec le **testeur de mot de passe WiFi**
- Mise à jour basée sur **les tentatives de connexion**
- Système de **suivi des adresses MAC**
- Affichage des **statuts en temps réel**

----------

### Gestion des erreurs

- Vérification de **l'initialisation de l’écran**
- Contrôle de **la communication SPI**
- Gestion de **l’allocation mémoire**
- Validation de la **création des tâches**

----------

### Exemple d'utilisation

```cpp
// Initialisation de l’écran
initializeTFT();

// Mise à jour du statut de connexion
updateConnectionScore("wifi_m2dfs_test", true, false);

// Ajout d’une adresse MAC
updateMACList("AA:BB:CC:DD:EE:FF", currentMillis);

```

----------

### Dépendances

- `Adafruit_GFX`
- `Adafruit_ILI9341`
- `QRCodeGenerator`
- `ESP32 SPI`

## Dépendances du projet

- Adafruit BusIO 1.17.0
- Adafruit GFX Library 1.11.11
- Adafruit ILI9341 1.6.1
- Adafruit SH110X 2.1.11
- Adafruit SSD1306 2.5.13
- Adafruit STMPE610 1.1.6
- Adafruit TSC2007 1.1.2
- Adafruit TouchScreen 1.1.5
- NimBLE-Arduino 1.4.3
- QRCodeGenerator 0.0.1
- TFT_eSPI 2.5.43
- XPT2046_Touchscreen 1.4

## Licence

Pirate Licence
