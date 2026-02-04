#ifndef STATO_H
#define STATO_H

#include <Arduino.h>

class Stato {
public:
    enum Livello {
        NORMALE,    // Verde: Tutto OK
        ATTENZIONE, // Giallo: Allarme Temperatura
        PERICOLO    // Rosso: Errore Hardware o Disconnessione
    };

    Stato();
    void begin();
    void imposta(Livello l);
    
private:
    Livello _livelloCorrente;
    void aggiornaLed();
};

#endif