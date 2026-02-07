

// Progetto Bracciale
// di Kicco972, 2025
// Google Gemini Pro

// --- INCLUSIONE LIBRERIE ---

// Includi i moduli del progetto
#include "mbed.h"            // Mbed OS (Sistema operativo real-time sottostante)
#include "Display.h"         // Gestione interfaccia grafica e touch
#include "BleNetwork.h"      // Gestione comunicazioni Bluetooth Low Energy
#include "Imu3DVisualizer.h" // Visualizzazione 3D dell'orientamento
#include "Bussola.h"         // Visualizzazione della Bussola
#include "WiFiGiga.h"        // Gestione WiFi
#include "Stato.h"           // Includi la nuova gestione stati
#include "Memoria.h"         // Gestione archiviazione dati
#include <Arduino_GigaDisplayTouch.h>
#include <Arduino_GigaDisplay_GFX.h>

// --- OGGETTI GLOBALI ---

// Oggetto Display Globale (condiviso tra Display.cpp e Imu3DVisualizer.cpp)
GigaDisplay_GFX gigaDisplay;
GigaDisplayRGB rgb;         // Oggetto per controllare il LED RGB integrato nel display shield

// Crea gli oggetti globali per i moduli personalizzati
Display display;            // Gestisce UI e pulsanti
BleNetwork myNetwork;       // Gestisce connessioni BLE con Nano Sense e Nano IoT
WiFiGiga myWifi;            // Gestisce connessione WiFi
Imu3DVisualizer imuViz;     // Gestisce la grafica 3D dell'IMU locale
Bussola bussolaViz;         // Oggetto per la visualizzazione della Bussola
Stato gestioneStato;        // Oggetto per gestire il LED di stato
Memoria memoria;            // Oggetto per gestire l'archivio dati

// --- VARIABILI DI STATO ---

// Variabile di stato per la modalità di visualizzazione
bool imuMode = false;       // true = visualizza sfera 3D, false = schermata base
bool bussolaMode = false;   // Flag per modalità bussola (true = visualizza bussola)
bool memoryMode = false;    // Flag per modalità visualizzazione memoria
bool imuOk = false;         // Flag per tracciare lo stato dell'hardware IMU

// Variabili per il datalogger
unsigned long lastLogTime = 0;

// --- SETUP ---
void setup()
{
  // Inizializzazione comunicazione seriale per debug
  Serial.begin(115200);

  Serial.println("--- Avvio Bracciale Giga ---");

  // Inizializza i moduli hardware e software
  display.begin(); // Avvia display e touch
  myNetwork.begin(); // Avvia modulo BLE
  myWifi.begin(); // Avvia connessione WiFi
  gestioneStato.begin(rgb); // Avvia gestione LED passando l'oggetto RGB del display
  memoria.begin(); // Avvia il filesystem (QSPI Flash)

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
    // --- PULSANTE 1 (Alto-Sinistra) ---
    // Funzione Base: SCAN
    // Funzione Sub: INDIETRO (Universale)
    
    if (memoryMode) {
        if (memoria.isViewingFileContent()) {
            // Livello 3: Stiamo guardando il contenuto -> Torna alla Lista File
            memoria.closeFileView();
            
            // Ripristina pulsanti per lista file
            display.setButtonLabel(Display::BUTTON_SCAN, "Indietro");
            display.setButtonLabel(Display::BUTTON_IMU, "Su");
            display.setButtonLabel(Display::BUTTON_BUSSOLA, "Giu");
            display.setButtonLabel(Display::BUTTON_LED, "Apri");
            display.drawButtons();
            
            memoria.drawContent(gigaDisplay);
        } else if (memoria.isViewingFiles()) {
            // Livello 2: Stiamo guardando la lista file -> Torna alla Selezione Drive
            memoria.exitFileList();
            
            // Ripristina pulsanti per selezione drive
            display.setButtonLabel(Display::BUTTON_SCAN, "Indietro");
            display.setButtonLabel(Display::BUTTON_IMU, "Flash");
            display.setButtonLabel(Display::BUTTON_BUSSOLA, "USB");
            display.setButtonLabel(Display::BUTTON_LED, "Entra");
            display.drawButtons();
            
            memoria.drawContent(gigaDisplay);
        } else {
            // Livello 1: Stiamo guardando la Selezione Drive -> Torna alla Schermata Base
            Serial.println("Torno alla schermata Base");
            memoryMode = false;
            display.showBaseScreen();
            display.updateLedButton(myNetwork.getActuatorState());
        }
    } else if (imuMode || bussolaMode) {
        // Se siamo in una sottomaschera (IMU o Bussola), torna alla base
        Serial.println("Torno alla schermata Base");
        imuMode = false;
        bussolaMode = false;
        
        display.showBaseScreen();
        display.updateLedButton(myNetwork.getActuatorState());
    } else {
        // Siamo nella schermata base: Esegue SCAN
        Serial.println("Pulsante 'Scan' premuto!");
        myNetwork.startScan();
    }
    break;

  case Display::BUTTON_IMU:
    // --- PULSANTE 2 ---
    // Funzione Base: Apri IMU
    // Funzione Sub: Disponibile
    
    if (imuMode || bussolaMode || memoryMode) {
        // Pulsante disponibile nelle sottomaschere
        if (memoryMode) {
            if (memoria.isViewingFileContent()) {
                memoria.scrollFileContent(-1); // Scroll Su
                memoria.drawContent(gigaDisplay);
            } else if (memoria.isViewingFiles()) {
                memoria.moveFileSelection(-1); // Selezione Su
                memoria.drawContent(gigaDisplay);
            } else {
            // Funzione: Seleziona FLASH
            memoria.selectDrive(0);
            memoria.drawContent(gigaDisplay);
            }
        }
    } else {
        // Entra in modalità IMU
        Serial.println("Apro schermata IMU");
        imuMode = true;
        display.prepareSubScreen();
        imuViz.drawBackground();
    }
    break;

  case Display::BUTTON_BUSSOLA:
    // --- PULSANTE 3 ---
    // Funzione Base: Apri BUSSOLA
    // Funzione Sub: Disponibile
    
    if (imuMode || bussolaMode || memoryMode) {
        if (memoryMode) {
            if (memoria.isViewingFileContent()) {
                memoria.scrollFileContent(1); // Scroll Giu
                memoria.drawContent(gigaDisplay);
            } else if (memoria.isViewingFiles()) {
                memoria.moveFileSelection(1); // Selezione Giu
                memoria.drawContent(gigaDisplay);
            } else {
            // Funzione: Seleziona USB
            memoria.selectDrive(1);
            memoria.drawContent(gigaDisplay);
            }
        }
    } else {
        // Entra in modalità Bussola
        Serial.println("Apro schermata Bussola");
        bussolaMode = true;
        display.prepareSubScreen();
        bussolaViz.drawBackground();
    }
    break;

  case Display::BUTTON_LED:
    // --- PULSANTE 4 ---
    // Funzione Base: Toggle LED Remoto
    // Funzione Sub: Disponibile
    
    if (imuMode || bussolaMode || memoryMode) {
        if (memoryMode) {
            if (memoria.isViewingFileContent()) {
                // Pulsante non usato o Page Down
            } else if (memoria.isViewingFiles()) {
                memoria.openSelectedFile();
                // Imposta pulsanti per visualizzazione file
                display.setButtonLabel(Display::BUTTON_SCAN, "Indietro");
                display.setButtonLabel(Display::BUTTON_IMU, "Su");
                display.setButtonLabel(Display::BUTTON_BUSSOLA, "Giu");
                display.setButtonLabel(Display::BUTTON_LED, "");
                display.drawButtons();
                memoria.drawContent(gigaDisplay);
            } else {
            // Funzione: ENTRA
            memoria.enterSelectedDrive();
            
            // Pulisci etichette pulsanti (non servono nella lista file per ora)
            display.setButtonLabel(Display::BUTTON_SCAN, "Indietro"); // Assicura che il tasto Indietro sia visibile
            display.setButtonLabel(Display::BUTTON_IMU, "Su");
            display.setButtonLabel(Display::BUTTON_BUSSOLA, "Giu");
            display.setButtonLabel(Display::BUTTON_LED, "Apri");
            display.drawButtons();
            
            memoria.drawContent(gigaDisplay);
            }
        }
    } else {
        // Azione LED Base
        Serial.println("Pulsante 'LED' premuto!");
        myNetwork.toggleActuator();
        display.updateLedButton(myNetwork.getActuatorState());
    }
    break;

  case Display::BUTTON_F1:
    // --- PULSANTE 5 (Basso-Sinistra) ---
    // Funzione Base: Apri MEMORIA
    // Funzione Sub: Disponibile
    
    if (!imuMode && !bussolaMode && !memoryMode) {
        // Entra in modalità Memoria
        Serial.println("Apro schermata Memoria");
        memoryMode = true;
        display.prepareSubScreen();
        
        // Imposta etichette pulsanti per navigazione
        display.setButtonLabel(Display::BUTTON_IMU, "Flash");
        display.setButtonLabel(Display::BUTTON_BUSSOLA, "USB");
        display.setButtonLabel(Display::BUTTON_LED, "Entra");
        display.drawButtons(); // Ridisegna per mostrare le nuove etichette
        
        // Contenuto
        memoria.drawContent(gigaDisplay);
    }
    break;

  case Display::NONE:
    // Nessun pulsante premuto, non fare nulla
    break;
  }

  // 3. Aggiorna la logica di rete (gestisce connessioni, riceve dati, riconnessioni, etc.)
  myNetwork.update();
  myWifi.update();            // Gestisce la riconnessione WiFi automatica

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
  else if (memoryMode)
  {
    // --- MODALITÀ MEMORIA ---
    // Schermata statica, non richiede aggiornamenti continui nel loop
  }
  else
  {
    // --- MODALITÀ BASE ---
    // Mostra stato connessioni e dati ambientali
    display.updateStatus(myNetwork.isScanning(), myNetwork.isSenseConnected(), myNetwork.isIoTConnected());
    
    // Aggiorna stato WiFi (IP e RSSI)
    display.updateWifiStatus(myWifi.isConnected(), myWifi.getIP(), myWifi.getRSSI());
    
    // Aggiorna Orologio (Data e Ora in alto a sinistra)
    display.updateClock(myWifi.getTimeString(), myWifi.getDateString());

    // Aggiorna la temperatura/umidità/pressione sul display se connesso a Sense
    if (myNetwork.isSenseConnected())
    {
      display.updateTemperature(myNetwork.getLatestTemperature());
      display.updateHumidity(myNetwork.getLatestHumidity());
      display.updatePressure(myNetwork.getLatestPressure());
    }
  }

  // 7. Gestione Archiviazione Dati (Ogni ora)
  // 3600000 ms = 1 ora
  if (millis() - lastLogTime > 3600000)
  {
      Serial.println("--- Tentativo Archiviazione Automatica (Oraria) ---");

      // Salva solo se abbiamo dati validi dal sensore Sense
      if (myNetwork.isSenseConnected()) {
          bool esito = memoria.logData(myWifi.getDateString(), 
                          myWifi.getTimeString(), 
                          myNetwork.getLatestTemperature(), 
                          myNetwork.getLatestHumidity(), 
                          myNetwork.getLatestPressure());
          
          if (esito) {
              Serial.println("Dati salvati correttamente su USB.");
          } else {
              Serial.println("Errore salvataggio: Chiavetta USB non trovata o errore scrittura.");
          }
      } else {
          Serial.println("Archiviazione saltata: Sensore Sense non connesso (Dati non validi).");
      }
      lastLogTime = millis();
  }

  // 8. Gestione Stato Sistema (LED RGB) con criteri aggiornati

  // Recupera temperatura corrente per controllo allarmi
  float temp = myNetwork.getLatestTemperature();
  // Definisci soglie di allarme (es. sotto 10°C o sopra 30°C)
  bool allarmeTemp = (temp < 10.0 || temp > 30.0) && (temp != 0.0);

  uint16_t coloreStato = VERDE; // Default

  // Logica prioritaria per determinare il colore dello stato
  if (!imuOk)
  {
    // Priorità 1: Errore Hardware -> Rosso (Pericolo)
    gestioneStato.imposta(Stato::PERICOLO);
    coloreStato = ROSSO;
  }
  else if (!myNetwork.isSenseConnected() || !myNetwork.isIoTConnected() || !myWifi.isConnected() || allarmeTemp)
  {
    // Priorità 2: Connessioni perse o Allarme Temp -> Giallo (Attenzione)
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

}