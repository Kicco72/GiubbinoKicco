#include "Stato.h"

Stato::Stato() : _livelloCorrente(PERICOLO), _rgb(NULL), _lastBlinkTime(0), _ledOn(false) {}

void Stato::begin(GigaDisplayRGB &rgb)
{
  _rgb = &rgb;
  _rgb->begin(); // Inizializza il LED RGB del display

  _lastBlinkTime = millis();
  _ledOn = true;
  accendiLed();

  // Stato iniziale di default
  imposta(PERICOLO);
}

void Stato::imposta(Livello l)
{
  // Aggiorna solo se lo stato cambia per evitare sfarfallii
  if (_livelloCorrente != l)
  {
    _livelloCorrente = l;
    // Reset del ciclo di lampeggio per feedback immediato
    _ledOn = true;
    _lastBlinkTime = millis();
    accendiLed();
  }
}

void Stato::update()
{
  if (!_rgb)
    return;

  unsigned long currentMillis = millis();

  if (_ledOn)
  {
    // ON per 300ms
    if (currentMillis - _lastBlinkTime >= 300)
    {
      _rgb->off();
      _ledOn = false;
      _lastBlinkTime = currentMillis;
    }
  }
  else
  {
    // OFF per 700ms
    if (currentMillis - _lastBlinkTime >= 700)
    {
      accendiLed();
      _ledOn = true;
      _lastBlinkTime = currentMillis;
    }
  }
}

void Stato::accendiLed()
{
  if (!_rgb)
    return;

  switch (_livelloCorrente)
  {
  case NORMALE:
    // Verde
    _rgb->on(0, 255, 0);
    break;
  case ATTENZIONE:
    // Giallo (Rosso + Verde)
    _rgb->on(255, 255, 0);
    break;
  case PERICOLO:
    // Rosso
    _rgb->on(255, 0, 0);
    break;
  }
}