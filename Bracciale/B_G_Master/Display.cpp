#include "Display.h"

// Definizione dei colori
#define CIANO   0x07FF
#define ROSSO   0xf800
#define BLE     0x001F
#define VERDE   0x07E0
#define MAGENTA 0xF81F
#define BIANCO  0xffff
#define NERO    0x0000
#define GIALLO  0xFFE0

// Definizioni dei pulsanti (posizione, dimensione, etichetta)
namespace {
    // Struttura per contenere le proprietà di un pulsante
    struct Button {
        Display::ButtonId id;
        int x, y, w, h;
        const char* label;
    };

    // Array con i nostri pulsanti
    Button buttons[] = {
        {Display::BUTTON_SCAN,   40, 300, 180, 80, "Scan"},
        {Display::BUTTON_STATUS, 260, 300, 180, 80, "Status"}
    };
    const int NUM_BUTTONS = sizeof(buttons) / sizeof(Button);
}

Display::Display() : _lastStatusMessage("") {}

void Display::begin() {
    _display.begin();
    _touchDetector.begin();
    showBaseScreen();
}

void Display::showBaseScreen() {
    _display.fillScreen(NERO);
    _display.setTextColor(BIANCO);
    _display.setTextSize(4);
    _display.setCursor(80, 50);
    _display.println("Bracciale");

    _display.setTextSize(3);
    _display.setCursor(150, 120);
    _display.println("Attivo");

    // Disegna i pulsanti
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        _display.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
        _display.setTextSize(3);
        // Centra l'etichetta nel pulsante (approssimato)
        _display.setCursor(buttons[i].x + 35, buttons[i].y + 28);
        _display.print(buttons[i].label);
    }
}

Display::ButtonId Display::checkTouch() {
    // Controlla se lo schermo è stato premuto
    if (_touchDetector.isPressed()) {
        TouchData touch = _touchDetector.read();

        // I dati del tocco (touch.x, touch.y) sono già mappati sulle coordinate dello schermo
        
        // Controlla se il tocco è avvenuto all'interno di uno dei pulsanti
        for (int i = 0; i < NUM_BUTTONS; ++i) {
            bool insideX = (touch.x >= buttons[i].x) && (touch.x <= buttons[i].x + buttons[i].w);
            bool insideY = (touch.y >= buttons[i].y) && (touch.y <= buttons[i].y + buttons[i].h);

            if (insideX && insideY) {
                // Trovato! Aggiungiamo un feedback visivo.
                _display.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
                _display.setTextColor(BLACK);
                _display.setCursor(buttons[i].x + 35, buttons[i].y + 28);
                _display.print(buttons[i].label);
                
                delay(150); // Breve ritardo per mostrare l'effetto e per debounce
                
                // Ripristina l'aspetto del pulsante
                _display.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
                _display.setTextColor(BIANCO);
                 _display.setCursor(buttons[i].x + 35, buttons[i].y + 28);
                _display.print(buttons[i].label);

                return buttons[i].id; // Ritorna l'ID del pulsante premuto
            }
        }
    }
    
    return Display::NONE; // Nessun pulsante premuto
}

void Display::updateStatus(bool isScanning, bool isSenseConnected, bool isIoTConnected) {
    String newMessage;

    if (isScanning) {
        newMessage = "Scansione in corso...";
    } else {
        if (isSenseConnected && isIoTConnected) {
            newMessage = "Sistema Connesso";
        } else if (isSenseConnected) {
            newMessage = "Connesso a Sense";
        } else if (isIoTConnected) {
            newMessage = "Connesso a IoT";
        } else {
            newMessage = "Pronto. Premi 'Scan'.";
        }
    }

    // Aggiorna il display solo se il messaggio è cambiato per evitare sfarfallio
    if (newMessage != _lastStatusMessage) {
        // Definisce l'area in cui disegnare lo stato
        const int STATUS_X = 40;
        const int STATUS_Y = 220;
        const int STATUS_W = 400;
        const int STATUS_H = 40;

        // Pulisci l'area di stato
        _display.fillRect(STATUS_X, STATUS_Y, STATUS_W, STATUS_H, NERO);
        
        // Scrivi il nuovo messaggio
        _display.setTextColor(BIANCO);
        _display.setTextSize(3);
        _display.setCursor(STATUS_X, STATUS_Y + 5);
        _display.print(newMessage);
        
        // Memorizza il nuovo messaggio
        _lastStatusMessage = newMessage;
    }
}
