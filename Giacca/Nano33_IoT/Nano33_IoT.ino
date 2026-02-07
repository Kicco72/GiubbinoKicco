/*
  Nano33_IoT.ino
  Codice per Arduino Nano 33 IoT
  Funzione: Periferica Giga R1 che agisce come attuatore (LED) controllato dal Master
*/

#include <ArduinoBLE.h>

// UUID definiti nel Master (Custom Service)
const char* uuidService = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* uuidCharSwitch = "19B10001-E8F2-537E-4F6C-D104768A1214";

BLEService iotService(uuidService);
BLEByteCharacteristic switchCharacteristic(uuidCharSwitch, BLERead | BLEWrite);

const int ledPin = LED_BUILTIN;

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);

  // Inizializzazione BLE
  if (!BLE.begin()) {
    Serial.println("Errore: Impossibile avviare BLE!");
    while (1);
  }

  // Imposta il nome locale che il Master cerca ("NanoIoT")
  BLE.setLocalName("NanoIoT");
  
  // Imposta il servizio pubblicizzato
  BLE.setAdvertisedService(iotService);

  // Aggiungi la caratteristica
  iotService.addCharacteristic(switchCharacteristic);

  // Aggiungi il servizio
  BLE.addService(iotService);

  // Imposta valore iniziale (Spento)
  switchCharacteristic.writeValue(0);

  // Avvia la pubblicità
  BLE.advertise();

  Serial.println("NanoIoT pronto e in ascolto...");
}

void loop() {
  // Attendi connessione da un centrale
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connesso al centrale: ");
    Serial.println(central.address());

    while (central.connected()) {
      // Controlla se il valore è stato scritto dal Master
      if (switchCharacteristic.written()) {
        byte val = switchCharacteristic.value();
        
        if (val) {
          Serial.println("Comando ricevuto: ON");
          digitalWrite(ledPin, HIGH);
        } else {
          Serial.println("Comando ricevuto: OFF");
          digitalWrite(ledPin, LOW);
        }
      }
    }

    Serial.println("Disconnesso dal centrale.");
  }
}
