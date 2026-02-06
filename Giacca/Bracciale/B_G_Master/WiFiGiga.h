#ifndef WIFI_GIGA_H
#define WIFI_GIGA_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

class WiFiGiga {
public:
    WiFiGiga();
    void begin();           // Avvia la connessione
    void update();          // Gestisce la riconnessione automatica
    bool isConnected();     // Restituisce true se connesso
    void printStatus();     // Stampa IP e potenza segnale su Serial
    String getIP();         // Restituisce IP come stringa
    int getRSSI();          // Restituisce RSSI
    
    // Metodi per l'orario
    String getTimeString(); // Restituisce "HH:MM"
    String getDateString(); // Restituisce "DD/MM/YYYY"

private:
    const char* _ssid;
    const char* _pass;
    unsigned long _lastAttemptTime;
    bool _hardwareOk;
    
    // Variabili per NTP
    WiFiUDP _udp;
    unsigned long _lastNtpSyncTime;
    bool _ntpRequestSent;
    unsigned long _ntpRequestTime;
    
    void handleNtp(); // Gestisce la macchina a stati NTP
    void sendNTPPacket(IPAddress& address);
};

#endif