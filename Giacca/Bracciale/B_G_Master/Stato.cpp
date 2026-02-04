#include "Stato.h"

Stato::Stato() : _livelloCorrente(PERICOLO) {}

void Stato::begin()
{
    // I pin LEDR, LEDG, LEDB sono definiti nel core Arduino per GIGA R1
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);

    // Spegni tutto all'inizio (Active LOW: HIGH = OFF)
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    // Stato iniziale di default
    imposta(PERICOLO);
}

void Stato::imposta(Livello l)
{
    // Aggiorna solo se lo stato cambia per evitare sfarfallii
    if (_livelloCorrente != l)
    {
        _livelloCorrente = l;
        aggiornaLed();
    }
}

void Stato::aggiornaLed()
{
    // Reset (Spegni tutto prima di impostare il nuovo colore)
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    switch (_livelloCorrente)
    {
    case NORMALE:
        // Verde
        digitalWrite(LEDG, LOW);
        break;
    case ATTENZIONE:
        // Giallo (Rosso + Verde)
        digitalWrite(LEDR, LOW);
        digitalWrite(LEDG, LOW);
        break;
    case PERICOLO:
        // Rosso
        digitalWrite(LEDR, LOW);
        break;
    }
}