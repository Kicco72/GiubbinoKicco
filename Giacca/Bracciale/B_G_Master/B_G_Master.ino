

// Progetto Bracciale Giubbino
// Progetto Bracciale
// di Kicco972, 2025
// Google Gemini Pro

// --- INCLUSIONE LIBRERIE ---
// Includi i moduli del progetto
#include "mbed.h" // Mbed OS (Sistema operativo real-time sottostante)
#include "Display.h" // Gestione interfaccia grafica e touch
#include "BleNetwork.h" // Gestione comunicazioni Bluetooth Low Energy
#include "Imu3DVisualizer.h" // Visualizzazione 3D dell'orientamento
#include "Bussola.h" // Includi la nuova classe
#include "Stato.h" // Includi la nuova gestione stati
#include <Arduino_GigaDisplayTouch.h>
#include <Arduino_GigaDisplay_GFX.h>

// --- OGGETTI GLOBALI ---
// Oggetto Display Globale (condiviso tra Display.cpp e Imu3DVisualizer.cpp)
GigaDisplay_GFX gigaDisplay;
GigaDisplayRGB rgb; // Oggetto per controllare il LED RGB integrato nel display shield

// Crea gli oggetti globali per i moduli personalizzati
Display display; // Gestisce UI e pulsanti
BleNetwork myNetwork; // Gestisce connessioni BLE con Nano Sense e Nano IoT
Imu3DVisualizer imuViz; // Gestisce la grafica 3D dell'IMU locale
Bussola bussolaViz; // Oggetto per la visualizzazione della Bussola
Stato gestioneStato; // Oggetto per gestire il LED di stato

// --- VARIABILI DI STATO ---
// Variabile di stato per la modalità di visualizzazione
bool imuMode = false; // true = visualizza sfera 3D, false = schermata base
bool bussolaMode = false; // Flag per modalità bussola (true = visualizza bussola)
bool imuOk = false; // Flag per tracciare lo stato dell'hardware IMU

// --- SETUP ---
void setup()
{
  // Inizializzazione comunicazione seriale per debug
  Serial.begin(115200);
  // while (!Serial); // Decommentare se si riscontrano problemi con l'output seriale all'avvio

  Serial.println("--- Avvio Bracciale Giga ---");

  // Inizializza i moduli hardware e software
  display.begin(); // Avvia display e touch
  myNetwork.begin(); // Avvia modulo BLE
  gestioneStato.begin(rgb); // Avvia gestione LED passando l'oggetto RGB del display

  // Inizializza IMU (Sensore inerziale locale)
  if (!imuViz.begin())
  {
    Serial.println("Errore inizializzazione IMU!");
    imuOk = false; // Segnala errore hardware
  }
  else
  {
    imuOk = true; // Hardware OK
  }

  // Ripristina la schermata base (pulsanti e titolo) perché l'init IMU ha pulito lo schermo
  display.showBaseScreen();

  Serial.println("Setup completato. Premi 'Scan' per cercare i dispositivi.");
}

// --- LOOP PRINCIPALE ---
void loop()
{
  // 1. Controlla l'input dell'utente (touchscreen)
  // Restituisce l'ID del pulsante premuto o NONE
  Display::ButtonId pressedButton = display.checkTouch();

  // 2. Esegue un'azione in base all'input
  switch (pressedButton)
  {
  case Display::BUTTON_SCAN:
    if (imuMode) {
        // In modalità IMU, questo pulsante diventa "Tare"
        Serial.println("Pulsante 'Tare' premuto!");
        imuViz.tare();
    } else {
        // In modalità Base, è "Scan"
        // Avvia la ricerca dei dispositivi BLE (Nano Sense / Nano IoT)
        Serial.println("Pulsante 'Scan' premuto!");
        myNetwork.startScan(); // Avvia la scansione BLE
    }
    break;

  case Display::BUTTON_IMU:
    // Gestisce l'ingresso/uscita dalla modalità visualizzazione 3D IMU
    Serial.println("Pulsante 'IMU' premuto!");
    if (bussolaMode) break; // Ignora se siamo in bussola (o gestisci uscita)
    imuMode = !imuMode; // Alterna la modalità

    if (imuMode)
    {
      // Entra in modalità IMU: Cambia tasto in "Indietro"
      display.setButtonLabel(Display::BUTTON_SCAN, "Tare"); // Cambia Scan in Tare
      display.setButtonLabel(Display::BUTTON_IMU, "Indietro");
      gigaDisplay.fillScreen(0x0000); // Pulisci schermo (Nero)
      display.drawButtons();          // Disegna pulsanti aggiornati
      imuViz.drawBackground();        // Prepara sfondo IMU
      display.resetStateIcon();       // Forza ridisegno LED virtuale (icona stato)
    }
    else
    {
      // Torna alla base: Ripristina tasto "IMU"
      display.setButtonLabel(Display::BUTTON_SCAN, "Scan"); // Ripristina Scan
      display.setButtonLabel(Display::BUTTON_IMU, "IMU");
      display.showBaseScreen(); // Ripristina schermata base
      display.updateLedButton(myNetwork.getActuatorState()); // Ripristina stato colore LED
      // showBaseScreen chiama già resetStateIcon internamente o possiamo chiamarlo qui
    }
    break;

  case Display::BUTTON_BUSSOLA:
    // Gestisce l'ingresso/uscita dalla modalità Bussola
    Serial.println("Pulsante 'Bussola' premuto!");
    if (imuMode) break; // Ignora se siamo in IMU
    bussolaMode = !bussolaMode; // Alterna la modalità

    if (bussolaMode) {
        // Entra in modalità Bussola
        display.setButtonLabel(Display::BUTTON_BUSSOLA, "Indietro");
        gigaDisplay.fillScreen(0x0000); // Pulisci schermo
        bussolaViz.drawBackground(); // Disegna quadrante bussola
        display.drawButtons(); // Disegna i pulsanti DOPO lo sfondo per assicurarne la visibilità
        display.resetStateIcon(); // Ridisegna icona stato
    } else {
        // Torna alla base
        display.setButtonLabel(Display::BUTTON_BUSSOLA, "Bussola");
        display.showBaseScreen();
        display.updateLedButton(myNetwork.getActuatorState());
    }
    break;

  case Display::BUTTON_LED:
    // Invia comando al Nano IoT per accendere/spegnere il LED remoto
    Serial.println("Pulsante 'LED' premuto!");
    myNetwork.toggleActuator(); // Invia comando BLE
    display.updateLedButton(myNetwork.getActuatorState()); // Aggiorna colore pulsante (Giallo/Nero)
    break;

  case Display::NONE:
    // Nessun pulsante premuto, non fare nulla
    break;
  }

  // 3. Aggiorna la logica di rete (gestisce connessioni, riceve dati, riconnessioni, etc.)
  myNetwork.update();

  // Aggiorna il lampeggio del LED di stato
  gestioneStato.update();

  // 4. Aggiorna il display in base alla modalità corrente
  if (imuMode)
  {
    // --- MODALITÀ IMU ---
    // Disegna la sfera 3D ruotata in base ai dati dell'accelerometro locale
    imuViz.updateAndDraw();
  }
  else if (bussolaMode)
  {
    // --- MODALITÀ BUSSOLA ---
    // Recupera i dati magnetici dal Nano Sense via BLE
    float mx, my, mz;
    myNetwork.getLatestMag(mx, my, mz);
    // Aggiorna la grafica della bussola
    bussolaViz.updateAndDraw(mx, my, mz);
  }
  else
  {
    // --- MODALITÀ BASE ---
    // Mostra stato connessioni e dati ambientali
    display.updateStatus(myNetwork.isScanning(), myNetwork.isSenseConnected(), myNetwork.isIoTConnected());

    // Aggiorna la temperatura/umidità/pressione sul display se connesso a Sense
    if (myNetwork.isSenseConnected())
    {
      display.updateTemperature(myNetwork.getLatestTemperature());
      display.updateHumidity(myNetwork.getLatestHumidity());
      display.updatePressure(myNetwork.getLatestPressure());
    }
  }

  // 6. Aggiungiamo una logica per fermare la scansione una volta connessi a tutto
  if (myNetwork.isScanning() && myNetwork.isSenseConnected() && myNetwork.isIoTConnected())
  {
    Serial.println("Trovati e connessi a entrambi i dispositivi. Interrompo scansione.");
    myNetwork.stopScan();
  }

  // 7. Gestione Stato Sistema (LED RGB) con criteri aggiornati

  // Recupera temperatura corrente per controllo allarmi
  float temp = myNetwork.getLatestTemperature();
  // Definisci soglie di allarme (es. sotto 10°C o sopra 30°C)
  bool allarmeTemp = (temp < 10.0 || temp > 30.0) && (temp != 0.0);

  uint16_t coloreStato = VERDE; // Default

  // Logica prioritaria per determinare il colore dello stato
  if (!imuOk || !myNetwork.isSenseConnected() || !myNetwork.isIoTConnected())
  {
    // Priorità 1: Se qualcosa non funziona o non è connesso -> Rosso (Pericolo)
    gestioneStato.imposta(Stato::PERICOLO);
    coloreStato = ROSSO;
  }
  else if (allarmeTemp)
  {
    // Priorità 2: Tutto connesso ma temperatura fuori range -> Giallo (Attenzione)
    gestioneStato.imposta(Stato::ATTENZIONE);
    coloreStato = GIALLO;
  }
  else
  {
    // Priorità 3: Tutto funziona e temperatura OK -> Verde (Normale)
    gestioneStato.imposta(Stato::NORMALE);
    coloreStato = VERDE;
  }

  // Aggiorna l'icona sul display (LED Virtuale in alto a destra)
  display.updateStateIcon(coloreStato);

  // Rimosso delay(20) per rendere l'animazione 3D più fluida
  // Il touch ha già il suo debounce interno o gestito in checkTouch
}