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
        BUTTON_STATUS,
        BUTTON_LED // Nuovo pulsante per il LED
    };

    Display();
    void begin();
    void showBaseScreen();
    ButtonId checkTouch();
    
    void updateStatus(bool isScanning, bool isSenseConnected, bool isIoTConnected);
    void updateTemperature(float temp);

private:
    Arduino_GigaDisplayTouch _touchDetector;
    String _lastStatusMessage;
    float _lastTempDisplayed;
};

#endif