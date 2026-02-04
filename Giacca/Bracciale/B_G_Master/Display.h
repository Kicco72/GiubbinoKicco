#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino_GigaDisplay.h>
#include <Arduino_GigaDisplayTouch.h>

class Display {
public:
    // Enum per identificare quale pulsante è stato premuto
    enum ButtonId {
        NONE,
        BUTTON_SCAN,
        BUTTON_STATUS
    };

    Display();
    void begin();
    void showBaseScreen();
    ButtonId checkTouch();
    void updateStatus(bool isScanning, bool isSenseConnected, bool isIoTConnected);

private:
    Arduino_GigaDisplay _display;
    Arduino_GigaDisplayTouch _touchDetector;
    String _lastStatusMessage; // Per evitare sfarfallio
};

#endif // DISPLAY_H 