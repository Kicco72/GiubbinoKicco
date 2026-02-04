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
        BUTTON_LED // Nuovo pulsante per il LED
    };

    Display();
    void begin();
    void showBaseScreen();
    ButtonId checkTouch();
    void drawButtons(); // Metodo reso pubblico per ridisegnare i pulsanti in altre schermate
    void setButtonLabel(ButtonId id, const char* label); // Nuovo metodo per cambiare etichetta
    
    void updateStatus(bool isScanning, bool isSenseConnected, bool isIoTConnected);
    void updateTemperature(float temp);
    void updateHumidity(float hum);
    
    void updateStateIcon(uint16_t color); // Disegna il LED virtuale
    void resetStateIcon(); // Forza il ridisegno (utile al cambio schermata)

private:
    Arduino_GigaDisplayTouch _touchDetector;
    String _lastStatusMessage;
    float _lastTempDisplayed;
    float _lastHumDisplayed;
    uint16_t _lastStateColor;
};

#endif