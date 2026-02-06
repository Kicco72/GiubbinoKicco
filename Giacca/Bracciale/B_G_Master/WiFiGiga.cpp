#include "WiFiGiga.h"
#include <mbed.h> // Per set_time()

// --- CONFIGURAZIONE RETE ---
// INSERISCI QUI I DATI DELLA TUA RETE DI CASA
const char* WIFI_SSID = "TP-Paradiso";
const char* WIFI_PASS = "Dario2001";

WiFiGiga::WiFiGiga() {
    _ssid = WIFI_SSID;
    _pass = WIFI_PASS;
    _lastAttemptTime = 0;
    _hardwareOk = false;
    _lastNtpSyncTime = 0;
    _ntpRequestSent = false;
    _ntpRequestTime = 0;
}

void WiFiGiga::begin() {
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
    
    // Avvia UDP per NTP sulla porta 2390
    _udp.begin(2390);
    
    _lastAttemptTime = millis();
}

void WiFiGiga::update() {
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
    
    // Gestione sincronizzazione orario NTP
    handleNtp();
}

bool WiFiGiga::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

void WiFiGiga::printStatus() {
    Serial.print("  SSID: "); Serial.println(WiFi.SSID());
    Serial.print("  IP:   "); Serial.println(WiFi.localIP());
    Serial.print("  RSSI: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
}

String WiFiGiga::getIP() {
    if (isConnected()) {
        IPAddress ip = WiFi.localIP();
        return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    }
    return "---";
}

int WiFiGiga::getRSSI() {
    if (isConnected()) return (int)WiFi.RSSI();
    return 0;
}

// --- GESTIONE NTP E ORARIO ---

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

void WiFiGiga::handleNtp() {
    if (!isConnected()) return;

    unsigned long currentMillis = millis();

    // Sincronizza ogni ora (3600000 ms) o se non è mai stato sincronizzato (_lastNtpSyncTime == 0)
    if (currentMillis - _lastNtpSyncTime > 3600000 || _lastNtpSyncTime == 0) {
        if (!_ntpRequestSent) {
            // Invia richiesta
            IPAddress timeServerIP;
            WiFi.hostByName("pool.ntp.org", timeServerIP);
            sendNTPPacket(timeServerIP);
            _ntpRequestSent = true;
            _ntpRequestTime = currentMillis;
            Serial.println("NTP: Richiesta inviata...");
        } else {
            // Attendi risposta (Timeout 1 secondo)
            if (currentMillis - _ntpRequestTime > 1000) {
                _ntpRequestSent = false; // Timeout, riproverà al prossimo ciclo
                Serial.println("NTP: Timeout.");
            } else {
                int cb = _udp.parsePacket();
                if (cb) {
                    // Risposta ricevuta!
                    _udp.read(packetBuffer, NTP_PACKET_SIZE);
                    
                    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
                    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
                    unsigned long secsSince1900 = highWord << 16 | lowWord;
                    const unsigned long seventyYears = 2208988800UL;
                    unsigned long epoch = secsSince1900 - seventyYears;

                    // Imposta RTC interno (Mbed OS)
                    // Aggiungi Offset Fuso Orario (Es. +1 ora = 3600 sec). 
                    // Per ora fisso a UTC+1 (CET).
                    set_time(epoch + 3600); 
                    
                    _lastNtpSyncTime = currentMillis;
                    _ntpRequestSent = false;
                    Serial.println("NTP: Orario sincronizzato!");
                }
            }
        }
    }
}

void WiFiGiga::sendNTPPacket(IPAddress& address) {
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Precision
    // 8 bytes zero per Root Delay & Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    _udp.beginPacket(address, 123);
    _udp.write(packetBuffer, NTP_PACKET_SIZE);
    _udp.endPacket();
}

String WiFiGiga::getTimeString() {
    time_t now = time(NULL);
    struct tm * timeinfo = localtime(&now);
    char buffer[10];
    strftime(buffer, 10, "%H:%M", timeinfo);
    return String(buffer);
}

String WiFiGiga::getDateString() {
    time_t now = time(NULL);
    struct tm * timeinfo = localtime(&now);
    char buffer[12];
    strftime(buffer, 12, "%d/%m/%Y", timeinfo);
    return String(buffer);
}