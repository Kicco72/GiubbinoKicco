

// Progetto Bracciale Giubbino
// di Kicco972, 2025
// Google Gemini Pro

// Includi i moduli del progetto
#include "Display.h"
#include "BleNetwork.h"
#include "Imu3DVisualizer.h"
#include <Arduino_GigaDisplayTouch.h>
#include <Arduino_GigaDisplay_GFX.h>

// Oggetto Display Globale (condiviso tra Display.cpp e Imu3DVisualizer.cpp)
GigaDisplay_GFX gigaDisplay;

// Crea gli oggetti globali per i moduli
Display display;
BleNetwork myNetwork;
Imu3DVisualizer imuViz;

// Variabile di stato per la modalità di visualizzazione
bool imuMode = false;

void setup()
{
  Serial.begin(115200);
  // while (!Serial); // Decommentare se si riscontrano problemi con l'output seriale all'avvio

  Serial.println("--- Avvio Bracciale Giga ---");

  // Inizializza i moduli
  display.begin();
  myNetwork.begin();

  // Inizializza IMU
  if (!imuViz.begin())
  {
    Serial.println("Errore inizializzazione IMU!");
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
    imuMode = !imuMode; // Alterna la modalità

    if (imuMode)
    {
      // Entra in modalità IMU: Cambia tasto in "Indietro"
      display.setButtonLabel(Display::BUTTON_IMU, "Indietro");
      gigaDisplay.fillScreen(0x0000); // Pulisci schermo
      display.drawButtons();          // Disegna pulsanti aggiornati
      imuViz.drawBackground();        // Prepara sfondo IMU
    }
    else
    {
      // Torna alla base: Ripristina tasto "IMU"
      display.setButtonLabel(Display::BUTTON_IMU, "IMU");
      display.showBaseScreen(); // Ripristina schermata base
    }
    break;

  case Display::BUTTON_LED:
    Serial.println("Pulsante 'LED' premuto!");
    myNetwork.toggleActuator();
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
  else
  {
    // --- MODALITÀ BASE ---
    display.updateStatus(myNetwork.isScanning(), myNetwork.isSenseConnected(), myNetwork.isIoTConnected());

    // Aggiorna la temperatura sul display se connesso a Sense
    if (myNetwork.isSenseConnected())
    {
      display.updateTemperature(myNetwork.getLatestTemperature());
      display.updateHumidity(myNetwork.getLatestHumidity());
    }
  }

  // 6. Aggiungiamo una logica per fermare la scansione una volta connessi
  if (myNetwork.isScanning() && myNetwork.isSenseConnected() && myNetwork.isIoTConnected())
  {
    Serial.println("Trovati e connessi a entrambi i dispositivi. Interrompo scansione.");
    myNetwork.stopScan();
  }

  // Rimosso delay(20) per rendere l'animazione 3D più fluida
  // Il touch ha già il suo debounce interno o gestito in checkTouch
}