#ifndef STATO_H
#define STATO_H

#include <Arduino.h>
#include <Arduino_GigaDisplay.h> // Necessario per GigaDisplayRGB

class Stato {
public:
    enum Livello {
        NORMALE,    // Verde: Tutto OK
        ATTENZIONE, // Giallo: Allarme Temperatura
        PERICOLO    // Rosso: Errore Hardware o Disconnessione
    };

    Stato();
    void begin(GigaDisplayRGB& rgb); // Modificato per accettare l'oggetto RGB
    void imposta(Livello l);
    void update();
    
private:
    Livello _livelloCorrente;
    GigaDisplayRGB* _rgb; // Puntatore all'oggetto RGB del display
    unsigned long _lastBlinkTime;
    bool _ledOn;
    void accendiLed();
};

#endif