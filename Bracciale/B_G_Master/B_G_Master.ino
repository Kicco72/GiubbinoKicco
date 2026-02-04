
// Includi librerie e files di intestazione necessari

//#include "Comunicazione.h"
#include "Ble.h"
#include "Imu3DVisualizer.h"
#include <Arduino_GigaDisplay_GFX.h>
#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_H7_Video.h"
#include "lvgl.h"


// Dichiarazioni globali

// Comunicazione
char String [Comunicazione()];

// 1. Instantiate the display OBJECT here (global)
GigaDisplay_GFX gigaDisplay;

// 2. Instantiate your visualizer
Imu3DVisualizer visualizer;

Arduino_H7_Video Display(800, 480, GigaDisplayShield);

Arduino_GigaDisplayTouch TouchDetector;


void setup() {

  Serial.begin(115200);    // Inizializza la comunicazione seriale
  ble.begin();             // Inizializza il modulo BLE
  gigaDisplay.begin();     // Inizializza Display first!
  visualizer.begin();      // Inizializza Visualizer
  Display.begin();         // Inizializza Display
  TouchDetector.begin();   // Inizializza Touch Detector
  myNetwork.begin();       // Inizializza BLE   BleNetwork

  //Display & Grid Setup
  lv_obj_t* screen = lv_obj_create(lv_scr_act());
  lv_obj_set_size(screen, Display.width(), Display.height());

  static lv_coord_t col_dsc[] = { 370, 370, LV_GRID_TEMPLATE_LAST };
  static lv_coord_t row_dsc[] = { 215, 215, 215, 215, LV_GRID_TEMPLATE_LAST };

  lv_obj_t* grid = lv_obj_create(lv_scr_act());
  lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
  lv_obj_set_size(grid, Display.width(), Display.height());

  //top left
  lv_obj_t* obj;
  obj = lv_obj_create(grid);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,  //column
                       LV_GRID_ALIGN_STRETCH, 0, 1);      //row

  //bottom left
  obj = lv_obj_create(grid);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,  //column
                       LV_GRID_ALIGN_STRETCH, 1, 1);      //row
  //top right
  obj = lv_obj_create(grid);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,  //column
                       LV_GRID_ALIGN_STRETCH, 0, 1);      //row

  //bottom right
  obj = lv_obj_create(grid);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,  //column
                       LV_GRID_ALIGN_STRETCH, 1, 1);      //row

}

void loop() {

  // 1. Mantiene viva la rete (fa tutto lui: scansione, riconnessione, lettura dati)
  myNetwork.update();

  // 2. Logica applicativa (esempio timer)
  if (millis() - previousMillis >= 5000) {
    previousMillis = millis();

    // Esempio: Controlla se siamo connessi
    if (myNetwork.isSenseConnected() && myNetwork.isIoTConnected()) {
      Serial.print("Sistema Online. Temp Attuale: ");
      Serial.println(myNetwork.getLatestTemperature());
      
      // Invia comando all'altra scheda
      myNetwork.toggleActuator();
    } else {
      Serial.println("In attesa delle connessioni...");
    }
  }

  Serial.println(" Prova di funzionamento");

  Serial.println(**Comunicazione()**);  // Esegue la funzione dal modulo Comunicazione (attualmente vuota)
  Serial.println("-------------------");

  lv_timer_handler();               // Gestione LVGL

  // visualizer.updateAndDraw();    // Aggiorna e disegna il visualizzatore

  delay(500);
}
