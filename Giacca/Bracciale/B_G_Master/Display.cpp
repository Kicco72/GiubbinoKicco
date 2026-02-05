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
#define GRIGIO_CHIARO 0x6666
#define GRIGIO_SCURO 0x4444

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

    // Layout Landscape (800x480)
    // Pulsanti in basso (Y=380), Centrati orizzontalmente
    Button buttons[] = {
        {Display::BUTTON_SCAN, 25, 380, 140, 80, "Scan"},
        {Display::BUTTON_IMU, 215, 380, 140, 80, "IMU"},
        {Display::BUTTON_BUSSOLA, 405, 380, 140, 80, "Bussola"},
        {Display::BUTTON_LED, 595, 380, 140, 80, "LED"}};
    const int NUM_BUTTONS = sizeof(buttons) / sizeof(Button);
}

Display::Display() : _lastStatusMessage(""), _lastTempDisplayed(-999.0), _lastHumDisplayed(-999.0), _lastPressDisplayed(-999.0), _lastStateColor(0), _buttonPressed(false) {}

void Display::begin()
{
    gigaDisplay.begin();        // Usa l'oggetto globale
    gigaDisplay.setRotation(1); // Imposta orientamento Landscape (800x480)
    _touchDetector.begin();
    showBaseScreen();
}

void Display::showBaseScreen()
{
    gigaDisplay.fillScreen(NERO);
    gigaDisplay.setTextColor(BIANCO);
    gigaDisplay.setTextSize(3);
    gigaDisplay.setCursor(300, 30); // Centrato (800px)
    gigaDisplay.println("Kicco972.net");

    gigaDisplay.setTextSize(2);
    gigaDisplay.setCursor(320, 80);
    gigaDisplay.println("Sistema Attivo");

    drawButtons();
    resetStateIcon(); // Forza il ridisegno dell'icona di stato

    // Resetta la cache dei valori per forzare l'aggiornamento immediato dei dati
    _lastTempDisplayed = -999.0;
    _lastHumDisplayed = -999.0;
    _lastPressDisplayed = -999.0;
    _lastStatusMessage = ""; 
}

void Display::drawButtons()
{
    for (int i = 0; i < NUM_BUTTONS; ++i)
    {
        gigaDisplay.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
        gigaDisplay.setTextSize(2);
        // Centra l'etichetta nel pulsante (approssimato)
        gigaDisplay.setCursor(buttons[i].x + 20, buttons[i].y + 30);
        gigaDisplay.print(buttons[i].label);
    }
}

void Display::setButtonLabel(ButtonId id, const char *label)
{
    for (int i = 0; i < NUM_BUTTONS; ++i)
    {
        if (buttons[i].id == id)
        {
            buttons[i].label = label;
            break;
        }
    }
}

void Display::updateLedButton(bool isOn)
{
    for (int i = 0; i < NUM_BUTTONS; ++i)
    {
        if (buttons[i].id == BUTTON_LED)
        {
            uint16_t color = isOn ? GIALLO : NERO;
            uint16_t textColor = isOn ? NERO : BIANCO;

            gigaDisplay.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, color);
            gigaDisplay.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);

            gigaDisplay.setTextColor(textColor);
            gigaDisplay.setTextSize(2);
            gigaDisplay.setCursor(buttons[i].x + 20, buttons[i].y + 30);
            gigaDisplay.print(buttons[i].label);
            break;
        }
    }
}

Display::ButtonId Display::checkTouch()
{
    GDTpoint_t points[1];
    uint8_t contacts = _touchDetector.getTouchPoints(points);

    // Controlla se lo schermo è stato premuto
    if (contacts > 0)
    {
        // Se stiamo già gestendo una pressione (utente tiene premuto), usciamo subito per non bloccare
        if (_buttonPressed)
            return Display::NONE;

        // Recupera coordinate native (Portrait)
        uint16_t rawX = points[0].x;
        uint16_t rawY = points[0].y;

        // DEBUG: Decommenta se vuoi vedere le coordinate grezze sul monitor seriale
        // Serial.print("Raw: "); Serial.print(rawX); Serial.print(","); Serial.println(rawY);

        // Limita i valori per evitare underflow/overflow nella mappatura
        if (rawX > 480)
            rawX = 480;
        if (rawY > 800)
            rawY = 800;

        // Mappatura coordinate per Landscape (Rotation 1)
        // Correzione assi per rotazione 90° standard
        // X Visuale = Y Touch
        // Y Visuale = 480 - X Touch
        uint16_t x = rawY;
        uint16_t y = 480 - rawX;

        // Controlla se il tocco è avvenuto all'interno di uno dei pulsanti
        for (int i = 0; i < NUM_BUTTONS; ++i)
        {
            bool insideX = (x >= buttons[i].x) && (x <= buttons[i].x + buttons[i].w);
            bool insideY = (y >= buttons[i].y) && (y <= buttons[i].y + buttons[i].h);

            if (insideX && insideY)
            {
                // Trovato! Aggiungiamo un feedback visivo.
                gigaDisplay.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
                gigaDisplay.setTextColor(NERO);
                gigaDisplay.setTextSize(2);
                gigaDisplay.setCursor(buttons[i].x + 20, buttons[i].y + 30);
                gigaDisplay.print(buttons[i].label);

                delay(150); // Breve ritardo per mostrare l'effetto e per debounce

                // Ripristina l'aspetto del pulsante
                gigaDisplay.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, NERO); // Pulisci lo sfondo bianco
                gigaDisplay.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, BIANCO);
                gigaDisplay.setTextColor(BIANCO);
                gigaDisplay.setTextSize(2);
                gigaDisplay.setCursor(buttons[i].x + 20, buttons[i].y + 30);
                gigaDisplay.print(buttons[i].label);

                // Segnala che il pulsante è stato premuto e attendiamo il rilascio (gestito nel prossimo ciclo)
                _buttonPressed = true;
                return buttons[i].id; // Ritorna l'ID del pulsante premuto
            }
        }
    }
    else
    {
        // Nessun contatto: l'utente ha sollevato il dito, resettiamo il flag
        _buttonPressed = false;
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
        const int STATUS_Y = 330; // Sopra i pulsanti (Y=380)
        const int STATUS_W = 760; // Più largo per landscape
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
        gigaDisplay.fillRect(200, 130, 400, 50, NERO); // Centrato

        gigaDisplay.setTextColor(VERDE);
        gigaDisplay.setTextSize(3);
        gigaDisplay.setCursor(260, 140);
        gigaDisplay.print("Temp: ");
        gigaDisplay.print(temp, 1);
        gigaDisplay.print(" C");
    }
}

void Display::updateHumidity(float hum)
{
    // Aggiorna solo se cambia significativamente (0.1 %)
    if (abs(hum - _lastHumDisplayed) > 0.1)
    {
        _lastHumDisplayed = hum;

        // Area Umidità (sotto la temperatura)
        gigaDisplay.fillRect(200, 190, 400, 50, NERO); // Centrato, Y=190

        gigaDisplay.setTextColor(CIANO);
        gigaDisplay.setTextSize(3);
        gigaDisplay.setCursor(260, 200);
        gigaDisplay.print("Hum:  ");
        gigaDisplay.print(hum, 1);
        gigaDisplay.print(" %");
    }
}

void Display::updatePressure(float press)
{
    // Aggiorna solo se cambia significativamente (0.1 kPa)
    if (abs(press - _lastPressDisplayed) > 0.1)
    {
        _lastPressDisplayed = press;

        // Area Pressione (sotto l'umidità)
        gigaDisplay.fillRect(200, 250, 400, 50, NERO); // Centrato, Y=250

        gigaDisplay.setTextColor(MAGENTA); // Colore diverso per distinguere
        gigaDisplay.setTextSize(3);
        gigaDisplay.setCursor(260, 260);
        gigaDisplay.print("Pres: ");
        gigaDisplay.print(press, 1);
        gigaDisplay.print(" kPa");
    }
}

void Display::updateStateIcon(uint16_t color)
{
    // Aggiorna solo se il colore cambia per evitare sfarfallio
    if (_lastStateColor != color)
    {
        _lastStateColor = color;
        // Disegna un cerchio pieno in alto a destra come "LED virtuale"
        gigaDisplay.fillCircle(750, 40, 20, color);
        // Bordo bianco per visibilità
        gigaDisplay.drawCircle(750, 40, 20, BIANCO);
    }
}

void Display::resetStateIcon()
{
    _lastStateColor = 0x1234; // Imposta un valore impossibile per forzare l'aggiornamento
}
