// Progetto Bracciale Giubbino
// di Kicco972, 2025

// Includi i moduli del progetto
#include "Display.h"
#include "BleNetwork.h"

// Crea gli oggetti globali per i moduli
Display display;
BleNetwork myNetwork;

void setup() {
  Serial.begin(115200);
  // while (!Serial); // Decommentare se si riscontrano problemi con l'output seriale all'avvio

  Serial.println("--- Avvio Bracciale Giga ---");

  // Inizializza i moduli
  display.begin();
  myNetwork.begin();

  Serial.println("Setup completato. Premi 'Scan' per cercare i dispositivi.");
}

void loop() {
  // 1. Controlla l'input dell'utente (touchscreen)
  Display::ButtonId pressedButton = display.checkTouch();

  // 2. Esegue un'azione in base all'input
  switch (pressedButton) {
    case Display::BUTTON_SCAN:
      Serial.println("Pulsante 'Scan' premuto!");
      myNetwork.startScan(); // Avvia la scansione BLE
      break;

    case Display::BUTTON_STATUS:
      Serial.println("Pulsante 'Status' premuto!");
      // TODO: Mostrare una schermata di stato
      // display.showStatusScreen(myNetwork.isSenseConnected(), myNetwork.isIoTConnected());
      break;

    case Display::NONE:
      // Nessun pulsante premuto, non fare nulla
      break;
  }

  // 3. Aggiorna la logica di rete (gestisce connessioni, riceve dati, etc.)
  myNetwork.update();

  // 4. Aggiorna il display con lo stato corrente
  display.updateStatus(myNetwork.isScanning(), myNetwork.isSenseConnected(), myNetwork.isIoTConnected());

  // 5. Aggiungiamo una logica per fermare la scansione una volta connessi
  if (myNetwork.isScanning() && myNetwork.isSenseConnected() && myNetwork.isIoTConnected()) {
    Serial.println("Trovati e connessi a entrambi i dispositivi. Interrompo scansione.");
    myNetwork.stopScan();
  }

  delay(20); // Piccolo ritardo per stabilità generale
}