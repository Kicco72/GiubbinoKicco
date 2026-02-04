#include "Display.h"

// Definizione dei colori
#define CIANO 0x07FF
#define ROSSO 0xf800
#define BLE 0x001F
#define VERDE 0x07E0
#define MAGENTA 0xF81F
#define BIANCO 0xffff
#define NERO 0x0000
#define GIALLO 0xFFE0

// Definizioni dei pulsanti (posizione, dimensione, etichetta)
namespace
{
    // Struttura per contenere le proprietà di un pulsante
    struct Button
    {
        Display::ButtonId id;
        int x, y, w, h;
        const char *label;
    };

    // Spostiamo i pulsanti in basso (Y=650) per lasciare spazio alla sfera al centro (Y=400)
    // Array con i nostri pulsanti
    Button buttons[] = {
        {Display::BUTTON_SCAN, 20, 650, 140, 80, "Scan"},
        {Display::BUTTON_STATUS, 170, 650, 140, 80, "Info"},
        {Display::BUTTON_LED, 320, 650, 140, 80, "LED"}};
    const int NUM_BUTTONS = sizeof(buttons) / sizeof(Button);
}

Display::Display() : _lastStatusMessage(""), _lastTempDisplayed(-999.0) {}

void Display::begin()
{
    gigaDisplay.begin(); // Usa l'oggetto globale
    _touchDetector.begin();
    showBaseScreen();
}

void Display::showBaseScreen()
{
    gigaDisplay.fillScreen(NERO);
    gigaDisplay.setTextColor(BIANCO);
    gigaDisplay.setTextSize(4);
    gigaDisplay.setCursor(130, 30);
    gigaDisplay.println("Bracciale");

    gigaDisplay.setTextSize(2);
    gigaDisplay.setCursor(180, 80);
    gigaDisplay.println("Sistema Attivo");

    // Disegna i pulsanti
    for (int i = 0; i < NUM_BUTTONS; ++i)
    {
        gigaDisplay.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
        gigaDisplay.setTextSize(3);
        // Centra l'etichetta nel pulsante (approssimato)
        gigaDisplay.setCursor(buttons[i].x + 20, buttons[i].y + 30);
        gigaDisplay.print(buttons[i].label);
    }
}

Display::ButtonId Display::checkTouch()
{
    // Controlla se lo schermo è stato premuto
    if (_touchDetector.isPressed())
    {
        TouchData touch = _touchDetector.read();

        // I dati del tocco (touch.x, touch.y) sono già mappati sulle coordinate dello schermo

        // Controlla se il tocco è avvenuto all'interno di uno dei pulsanti
        for (int i = 0; i < NUM_BUTTONS; ++i)
        {
            bool insideX = (touch.x >= buttons[i].x) && (touch.x <= buttons[i].x + buttons[i].w);
            bool insideY = (touch.y >= buttons[i].y) && (touch.y <= buttons[i].y + buttons[i].h);

            if (insideX && insideY)
            {
                // Trovato! Aggiungiamo un feedback visivo.
                gigaDisplay.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
                gigaDisplay.setTextColor(NERO);
                gigaDisplay.setCursor(buttons[i].x + 20, buttons[i].y + 30);
                gigaDisplay.print(buttons[i].label);

                delay(150); // Breve ritardo per mostrare l'effetto e per debounce

                // Ripristina l'aspetto del pulsante
                gigaDisplay.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
                gigaDisplay.setTextColor(BIANCO);
                gigaDisplay.setCursor(buttons[i].x + 20, buttons[i].y + 30);
                gigaDisplay.print(buttons[i].label);

                return buttons[i].id; // Ritorna l'ID del pulsante premuto
            }
        }
    }

    return Display::NONE; // Nessun pulsante premuto
}

void Display::updateStatus(bool isScanning, bool isSenseConnected, bool isIoTConnected)
{
    String newMessage;

    if (isScanning)
    {
        newMessage = "Scansione in corso...";
    }
    else
    {
        if (isSenseConnected && isIoTConnected)
        {
            newMessage = "Sistema Connesso";
        }
        else if (isSenseConnected)
        {
            newMessage = "Connesso a Sense";
        }
        else if (isIoTConnected)
        {
            newMessage = "Connesso a IoT";
        }
        else
        {
            newMessage = "Pronto. Premi 'Scan'.";
        }
    }

    // Aggiorna il display solo se il messaggio è cambiato per evitare sfarfallio
    if (newMessage != _lastStatusMessage)
    {
        // Definisce l'area in cui disegnare lo stato
        const int STATUS_X = 20;
        const int STATUS_Y = 600; // Sopra i pulsanti
        const int STATUS_W = 440;
        const int STATUS_H = 40;

        // Pulisci l'area di stato
        gigaDisplay.fillRect(STATUS_X, STATUS_Y, STATUS_W, STATUS_H, NERO);

        // Scrivi il nuovo messaggio
        gigaDisplay.setTextColor(BIANCO);
        gigaDisplay.setTextSize(2);
        gigaDisplay.setCursor(STATUS_X, STATUS_Y + 10);
        gigaDisplay.print(newMessage);

        // Memorizza il nuovo messaggio
        _lastStatusMessage = newMessage;
    }
}

void Display::updateTemperature(float temp)
{
    // Aggiorna solo se cambia significativamente (0.1 gradi)
    if (abs(temp - _lastTempDisplayed) > 0.1)
    {
        _lastTempDisplayed = temp;

        // Area Temperatura (in alto a sinistra/centro)
        gigaDisplay.fillRect(20, 120, 440, 50, NERO);

        gigaDisplay.setTextColor(VERDE);
        gigaDisplay.setTextSize(4);
        gigaDisplay.setCursor(80, 130);
        gigaDisplay.print("Temp: ");
        gigaDisplay.print(temp, 1);
        gigaDisplay.print(" C");
    }
}
