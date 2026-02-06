#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Arduino_GigaDisplay_GFX.h>
#include <Arduino_GigaDisplayTouch.h>

// Riferimento all'oggetto display globale definito nel .ino
extern GigaDisplay_GFX gigaDisplay;

class Display {
public:
    enum ButtonId {
        NONE,
        BUTTON_SCAN,
        BUTTON_IMU, // Modificato da STATUS a IMU
        BUTTON_BUSSOLA, // Nuovo pulsante Bussola
        BUTTON_LED,
        BUTTON_F1,
        BUTTON_F2,
        BUTTON_F3,
        BUTTON_F4
    };

    Display();
    void begin();
    void showBaseScreen();
    void prepareSubScreen(); // Prepara il layout standard per le sottomaschere
    ButtonId checkTouch();
    void drawButtons(); // Metodo reso pubblico per ridisegnare i pulsanti in altre schermate
    void setButtonLabel(ButtonId id, const char* label); // Nuovo metodo per cambiare etichetta
    void updateLedButton(bool isOn); // Aggiorna il colore del pulsante LED
    
    void updateStatus(bool isScanning, bool isSenseConnected, bool isIoTConnected);
    void updateWifiStatus(bool isConnected, String ip, int rssi); // Nuovo metodo WiFi
    void updateClock(String timeStr, String dateStr); // Nuovo metodo Orologio
    void updateTemperature(float temp);
    void updateHumidity(float hum);
    void updatePressure(float press);
    
    void updateStateIcon(uint16_t color); // Disegna il LED virtuale
    void resetStateIcon(); // Forza il ridisegno (utile al cambio schermata)

private:
    Arduino_GigaDisplayTouch _touchDetector;
    String _lastStatusMessage;
    float _lastTempDisplayed;
    float _lastHumDisplayed;
    float _lastPressDisplayed;
    uint16_t _lastStateColor;
    bool _buttonPressed; // Flag per gestire il debounce senza bloccare
    bool _lastWifiConnected;
    String _lastIp;
    int _lastRssi;
    String _lastTimeDisplayed;
};

#endif