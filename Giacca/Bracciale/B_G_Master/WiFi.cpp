#include "wifi.h"




// --- CONFIGURAZIONE RETE ---
// INSERISCI QUI I DATI DELLA TUA RETE DI CASA
const char* WIFI_SSID = "TP-Paradiso";
const char* WIFI_PASS = "Dario2001";

WifiNetwork::WifiNetwork() {
    _ssid = WIFI_SSID;
    _pass = WIFI_PASS;
    _lastAttemptTime = 0;
    _hardwareOk = false;
}

void WifiNetwork::begin() {
    Serial.println("WiFi: Inizializzazione...");

    // Verifica presenza hardware
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("WiFi: Errore - Modulo non rilevato!");
        _hardwareOk = false;
        return;
    }
    _hardwareOk = true;

    // Verifica versione firmware
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("WiFi: Avviso - Consigliato aggiornamento firmware.");
    }

    Serial.print("WiFi: Tentativo di connessione a ");
    Serial.println(_ssid);

    // Avvia connessione
    WiFi.begin(_ssid, _pass);

    // Attesa connessione (Timeout 10 secondi)
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 10000) {
            Serial.println("\nWiFi: Timeout connessione.");
            break;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (isConnected()) {
        Serial.println("WiFi: Connesso con successo!");
        printStatus();
    }
    
    _lastAttemptTime = millis();
}

void WifiNetwork::update() {
    // Se l'hardware non c'è, inutile provare
    if (!_hardwareOk) return;

    // Se NON siamo connessi, proviamo a riconnetterci ogni 10 secondi
    if (WiFi.status() != WL_CONNECTED) {
        unsigned long currentMillis = millis();
        if (currentMillis - _lastAttemptTime >= 10000) {
            Serial.println("WiFi: Connessione persa o assente. Tentativo di riconnessione...");
            
            // Disconnetti per pulire lo stato precedente e riprova
            WiFi.disconnect();
            WiFi.begin(_ssid, _pass);
            
            _lastAttemptTime = currentMillis;
        }
    }
}

bool WifiNetwork::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

void WifiNetwork::printStatus() {
    Serial.print("  SSID: "); Serial.println(WiFi.SSID());
    Serial.print("  IP:   "); Serial.println(WiFi.localIP());
    Serial.print("  RSSI: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
}

String WifiNetwork::getIP() {
    if (isConnected()) {
        IPAddress ip = WiFi.localIP();
        return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    }
    return "---";
}

int WifiNetwork::getRSSI() {
    if (isConnected()) return (int)WiFi.RSSI();
    return 0;
}