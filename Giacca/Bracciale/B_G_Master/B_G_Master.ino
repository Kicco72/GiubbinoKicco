

// Progetto Bracciale Giubbino
// di Kicco972, 2025
// Google Gemini Pro

// Includi i moduli del progetto
#include "mbed.h" // Mbed OS
#include "Display.h"
#include "BleNetwork.h"
#include "Imu3DVisualizer.h"
#include "Bussola.h" // Includi la nuova classe
#include "Stato.h" // Includi la nuova gestione stati
#include <Arduino_GigaDisplayTouch.h>
#include <Arduino_GigaDisplay_GFX.h>

// Oggetto Display Globale (condiviso tra Display.cpp e Imu3DVisualizer.cpp)
GigaDisplay_GFX gigaDisplay;
GigaDisplayRGB rgb; // RGB Oggetto

// Crea gli oggetti globali per i moduli
Display display;
BleNetwork myNetwork;
Imu3DVisualizer imuViz;
Bussola bussolaViz; // Oggetto Bussola
Stato gestioneStato; // Oggetto per gestire il LED

// Variabile di stato per la modalità di visualizzazione
bool imuMode = false;
bool bussolaMode = false; // Flag per modalità bussola
bool imuOk = false; // Flag per tracciare lo stato dell'hardware IMU

void setup()
{
  Serial.begin(115200);
  // while (!Serial); // Decommentare se si riscontrano problemi con l'output seriale all'avvio

  Serial.println("--- Avvio Bracciale Giga ---");

  // Inizializza i moduli
  display.begin();
  myNetwork.begin();
  gestioneStato.begin(rgb); // Avvia gestione LED passando l'oggetto RGB del display

  // Inizializza IMU
  if (!imuViz.begin())
  {
    Serial.println("Errore inizializzazione IMU!");
    imuOk = false;
  }
  else
  {
    imuOk = true;
  }

  // Ripristina la schermata base (pulsanti e titolo) perché l'init IMU ha pulito lo schermo
  display.showBaseScreen();

  Serial.println("Setup completato. Premi 'Scan' per cercare i dispositivi.");
}

void loop()
{
  // 1. Controlla l'input dell'utente (touchscreen)
  Display::ButtonId pressedButton = display.checkTouch();

  // 2. Esegue un'azione in base all'input
  switch (pressedButton)
  {
  case Display::BUTTON_SCAN:
    Serial.println("Pulsante 'Scan' premuto!");
    myNetwork.startScan(); // Avvia la scansione BLE
    break;

  case Display::BUTTON_IMU:
    Serial.println("Pulsante 'IMU' premuto!");
    if (bussolaMode) break; // Ignora se siamo in bussola (o gestisci uscita)
    imuMode = !imuMode; // Alterna la modalità

    if (imuMode)
    {
      // Entra in modalità IMU: Cambia tasto in "Indietro"
      display.setButtonLabel(Display::BUTTON_IMU, "Indietro");
      gigaDisplay.fillScreen(0x0000); // Pulisci schermo
      display.drawButtons();          // Disegna pulsanti aggiornati
      imuViz.drawBackground();        // Prepara sfondo IMU
      display.resetStateIcon();       // Forza ridisegno LED virtuale
    }
    else
    {
      // Torna alla base: Ripristina tasto "IMU"
      display.setButtonLabel(Display::BUTTON_IMU, "IMU");
      display.showBaseScreen(); // Ripristina schermata base
      display.updateLedButton(myNetwork.getActuatorState()); // Ripristina stato colore LED
      // showBaseScreen chiama già resetStateIcon internamente o possiamo chiamarlo qui
    }
    break;

  case Display::BUTTON_BUSSOLA:
    Serial.println("Pulsante 'Bussola' premuto!");
    if (imuMode) break; // Ignora se siamo in IMU
    bussolaMode = !bussolaMode;

    if (bussolaMode) {
        display.setButtonLabel(Display::BUTTON_BUSSOLA, "Indietro");
        gigaDisplay.fillScreen(0x0000);
        bussolaViz.drawBackground();
        display.drawButtons(); // Disegna i pulsanti DOPO lo sfondo per assicurarne la visibilità
        display.resetStateIcon();
    } else {
        display.setButtonLabel(Display::BUTTON_BUSSOLA, "Bussola");
        display.showBaseScreen();
        display.updateLedButton(myNetwork.getActuatorState());
    }
    break;

  case Display::BUTTON_LED:
    Serial.println("Pulsante 'LED' premuto!");
    myNetwork.toggleActuator();
    display.updateLedButton(myNetwork.getActuatorState()); // Aggiorna colore pulsante
    break;

  case Display::NONE:
    // Nessun pulsante premuto, non fare nulla
    break;
  }

  // 3. Aggiorna la logica di rete (gestisce connessioni, riceve dati, etc.)
  myNetwork.update();

  // 4. Aggiorna il display in base alla modalità corrente
  if (imuMode)
  {
    // --- MODALITÀ IMU ---
    imuViz.updateAndDraw();
  }
  else if (bussolaMode)
  {
    // --- MODALITÀ BUSSOLA ---
    float mx, my, mz;
    myNetwork.getLatestMag(mx, my, mz);
    bussolaViz.updateAndDraw(mx, my, mz);
  }
  else
  {
    // --- MODALITÀ BASE ---
    display.updateStatus(myNetwork.isScanning(), myNetwork.isSenseConnected(), myNetwork.isIoTConnected());

    // Aggiorna la temperatura sul display se connesso a Sense
    if (myNetwork.isSenseConnected())
    {
      display.updateTemperature(myNetwork.getLatestTemperature());
      display.updateHumidity(myNetwork.getLatestHumidity());
      display.updatePressure(myNetwork.getLatestPressure());
    }
  }

  // 6. Aggiungiamo una logica per fermare la scansione una volta connessi
  if (myNetwork.isScanning() && myNetwork.isSenseConnected() && myNetwork.isIoTConnected())
  {
    Serial.println("Trovati e connessi a entrambi i dispositivi. Interrompo scansione.");
    myNetwork.stopScan();
  }

  // 7. Gestione Stato Sistema (LED RGB) con criteri aggiornati

  // Recupera temperatura corrente
  float temp = myNetwork.getLatestTemperature();
  // Definisci soglie di allarme (es. sotto 10°C o sopra 30°C)
  bool allarmeTemp = (temp < 10.0 || temp > 30.0) && (temp != 0.0);

  uint16_t coloreStato = VERDE; // Default

  if (!imuOk || !myNetwork.isSenseConnected() || !myNetwork.isIoTConnected())
  {
    // Se qualcosa non funziona o non è connesso -> Rosso (Pericolo)
    gestioneStato.imposta(Stato::PERICOLO);
    coloreStato = ROSSO;
  }
  else if (allarmeTemp)
  {
    // Tutto connesso ma temperatura fuori range -> Giallo (Attenzione)
    gestioneStato.imposta(Stato::ATTENZIONE);
    coloreStato = GIALLO;
  }
  else
  {
    // Tutto funziona e temperatura OK -> Verde (Normale)
    gestioneStato.imposta(Stato::NORMALE);
    coloreStato = VERDE;
  }

  // Aggiorna l'icona sul display (LED Virtuale)
  display.updateStateIcon(coloreStato);

  // Rimosso delay(20) per rendere l'animazione 3D più fluida
  // Il touch ha già il suo debounce interno o gestito in checkTouch
}