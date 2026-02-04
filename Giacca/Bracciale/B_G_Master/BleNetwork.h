/*
  BleNetwork.h
  Gestisce la connessione multipla con Nano Sense e Nano IoT
*/
#ifndef BLE_NETWORK_H
#define BLE_NETWORK_H

#include <Arduino.h>
#include <ArduinoBLE.h>

class BleNetwork {
  public:
    // Costruttore
    BleNetwork();

    // Inizializzazione
    void begin();
    
    // Funzioni per il controllo manuale della scansione
    void startScan();
    void stopScan();
    bool isScanning();

    // Da chiamare nel loop() continuamente
    void update();

    // Metodi per interagire con i dati
    float getLatestTemperature();
    void toggleActuator();

    // Stato delle connessioni (per debug/led)
    bool isSenseConnected();
    bool isIoTConnected();

  private:
    // Oggetti per le periferiche
    BLEDevice _senseDevice;
    BLEDevice _iotDevice;

    // Variabili di stato
    bool _senseConnected;
    bool _iotConnected;
    bool _isScanning;
    
    // Dati salvati
    float _lastTemperature;
    bool _actuatorState;
    unsigned long _lastPollTime; // Per temporizzare la lettura

    // UUIDs
    const char* _uuidSenseService;
    const char* _uuidSenseCharTemp;
    const char* _uuidIoTService;
    const char* _uuidIoTCharSwitch;

    // Metodi interni (privati)
    void scanAndConnect();
    bool connectToSense(BLEDevice p);
    bool connectToIoT(BLEDevice p);
    void pollSense();
    void writeIoT(bool state);
};

#endif