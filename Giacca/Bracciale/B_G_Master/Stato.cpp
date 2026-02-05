#include "Stato.h"

Stato::Stato() : _livelloCorrente(PERICOLO), _rgb(NULL) {}

void Stato::begin(GigaDisplayRGB &rgb)
{
  _rgb = &rgb;
  _rgb->begin(); // Inizializza il LED RGB del display

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
  if (!_rgb)
    return;

  switch (_livelloCorrente)
  {
  case NORMALE:
    // Verde
    _rgb->on(0, 255, 0);
    delay(300);
    break;
  case ATTENZIONE:
    // Giallo (Rosso + Verde)
    _rgb->on(255, 255, 0);
    delay(300);
    break;
  case PERICOLO:
    // Rosso
    _rgb->on(255, 0, 0);
    delay(300);
    break;
  }
}