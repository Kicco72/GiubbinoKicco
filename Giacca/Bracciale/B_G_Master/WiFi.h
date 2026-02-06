#ifndef WIFI_NETWORK_H
#define WIFI_NETWORK_H

#include <Arduino.h>
#include <WiFi.h>

class WifiNetwork {
public:
    WifiNetwork();
    void begin();           // Avvia la connessione
    void update();          // Gestisce la riconnessione automatica
    bool isConnected();     // Restituisce true se connesso
    void printStatus();     // Stampa IP e potenza segnale su Serial
    String getIP();         // Restituisce IP come stringa
    int getRSSI();          // Restituisce RSSI

private:
    const char* _ssid;
    const char* _pass;
    unsigned long _lastAttemptTime;
    bool _hardwareOk;
};

#endif