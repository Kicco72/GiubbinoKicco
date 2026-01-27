
// GigaR1_pulsanti.ino
// Progetto di esempio per l'uso di pulsanti con LVGL su Giga R1
// Autore: Kicco972.net
// Data: 27/12/2025
// Versione: 1.0

// Inclusione delle librerie necessarie
#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_GigaDisplay_GFX.h"

// Dichiarazioni globali
Arduino_H7_Video Display(800, 480, GigaDisplayShield); // Create Display
Arduino_GigaDisplayTouch Tocco;                        // Tocco Clik
GigaDisplay_GFX Schermo;                               // Create Schermo

// Definizione dei colori
#define CIANO 0x07FF
#define ROSSO 0xf800
#define BLE 0x001F
#define VERDE 0x07E0
#define MAGENTA 0xF81F
#define BIANCO 0xffff
#define NERO 0x0000
#define GIALLO 0xFFE0
#define GRIGIO_CHIARO 0x6666

// Evento Pulsante 1
static void btn_event_cb(lv_event_t *e)
{
#if (LVGL_VERSION_MAJOR == 9)
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
#else
  lv_obj_t *btn = lv_event_get_target(e);
#endif
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  lv_label_set_text_fmt(label, "Tocco!");
}

// Evento Pulsante 2
static void btn1_event_cb(lv_event_t *e) // Modificato nome funzione per evitare conflitto
{
#if (LVGL_VERSION_MAJOR == 9)                          // Correzione per compatibilità LVGL 9
  lv_obj_t *btn1 = (lv_obj_t *)lv_event_get_target(e); // Correzione per compatibilità LVGL 9
#else
  lv_obj_t *btn1 = lv_event_get_target(e); // Correzione per compatibilità LVGL 9
#endif
  lv_obj_t *label = lv_obj_get_child(btn1, 0); // Correzione per compatibilità LVGL 9
  lv_label_set_text_fmt(label, "Tocco!");      // Correzione per compatibilità LVGL 9
}

// Evento Pulsante 3
static void btn2_event_cb(lv_event_t *e) // Modificato nome funzione per evitare conflitto
{
#if (LVGL_VERSION_MAJOR == 9)                          // Correzione per compatibilità LVGL 9
  lv_obj_t *btn2 = (lv_obj_t *)lv_event_get_target(e); // Correzione per compatibilità LVGL 9
#else
  lv_obj_t *btn2 = lv_event_get_target(e); // Correzione per compatibilità LVGL 9
#endif
  lv_obj_t *label = lv_obj_get_child(btn2, 0); // Correzione per compatibilità LVGL 9
  lv_label_set_text_fmt(label, "Tocco!");      // Correzione per compatibilità LVGL 9
}

void setup() // Setup
{
  // Inizializzazioni

  Serial.begin(115200); // Inizializzazione seriale
  Display.begin();      // Inizializzazione Display
  Schermo.begin();      // Inizializzazione Schermo
  Tocco.begin();        // Inizializzazione Tocco
  lv_init();            // Inizializzazione LVGL

  // Configurazione LVGL
  lv_obj_t *screen = lv_obj_create(lv_scr_act());             // Creazione schermo principale
  lv_obj_set_size(screen, Display.width(), Display.height()); // Impostazione dimensioni schermo

  static lv_coord_t col_dsc[] = {300, LV_GRID_TEMPLATE_LAST}; // Definizione colonne griglia
  static lv_coord_t row_dsc[] = {320, LV_GRID_TEMPLATE_LAST}; // Definizione righe griglia

  lv_obj_t *grid = lv_obj_create(lv_scr_act());             // Creazione griglia
  lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);        // Impostazione griglia
  lv_obj_set_size(grid, Display.width(), Display.height()); // Impostazione dimensioni griglia
  lv_obj_center(grid);                                      // Centratura griglia

  lv_obj_t *label; // Definizione label
  lv_obj_t *obj;   // Definizione oggetto

  obj = lv_obj_create(grid); // Creazione oggetto
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, 0, 1); // Posizionamento oggetto
  lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);    // Layout flessibile

  // Pulsante 1
  lv_obj_t *btn = lv_btn_create(obj);                             // Creazione pulsante
  lv_obj_set_size(btn, 200, 80);                                  // Impostazione dimensioni pulsante
  lv_obj_center(btn);                                             // Centratura pulsante
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL); // Aggiunta callback evento
  label = lv_label_create(btn);                                   // Creazione label
  lv_label_set_text(label, "Click me!");                          // Impostazione testo label
  lv_obj_center(label);                                           // Centratura label

  // Pulsante 2
  lv_obj_t *btn1 = lv_btn_create(obj);                             // Creazione pulsante
  lv_obj_set_size(btn1, 200, 80);                                  // Impostazione dimensioni pulsante
  lv_obj_center(btn1);                                             // Centratura pulsante
  lv_obj_add_event_cb(btn, btn1_event_cb, LV_EVENT_CLICKED, NULL); // Aggiunta callback evento
  label = lv_label_create(btn1);                                   // Creazione label
  lv_label_set_text(label, " IMU ");                               // Impostazione testo label
  lv_obj_center(label);                                            // Centratura label

  // Pulsante 3
  lv_obj_t *btn2 = lv_btn_create(obj);                             // Creazione pulsante
  lv_obj_set_size(btn2, 200, 80);                                  // Impostazione dimensioni pulsante
  lv_obj_center(btn2);                                             // Centratura pulsante
  lv_obj_add_event_cb(btn, btn2_event_cb, LV_EVENT_CLICKED, NULL); // Aggiunta callback evento
  label = lv_label_create(btn2);                                   // Creazione label
  lv_label_set_text(label, " 33 BLE ");                            // Impostazione testo label
  lv_obj_center(label);                                            // Centratura label

  // Impostazioni Schermo
  Schermo.setRotation(1);            // Rotazione schermo
  Schermo.fillScreen(GRIGIO_CHIARO); // Colore sfondo

  // Titolo
  Schermo.setCursor(500, 200);   // Posizione cursore
  Schermo.setTextSize(1);        // Dimensione testo
  Schermo.setTextColor(ROSSO);   // Colore testo
  Schermo.print("Kicco972.net"); // Stampa titolo
}

void loop()
{
  lv_timer_handler(); // Gestione timer LVGL
  delay(5);           // Piccola pausa
}
