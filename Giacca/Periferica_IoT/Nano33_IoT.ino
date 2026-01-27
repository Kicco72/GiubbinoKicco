/*
  NANO 33 IoT - PERIPHERAL
  Attuatore su PIN 4
  Progetto di kicco972.net
*/

#include <Arduino.h>
#include <ArduinoBLE.h>

const int ledPin = 4; // Pin da controllare

// UUID Custom per il servizio attuatore
BLEService actuatorService("19B10000-E8F2-537E-4F6C-D104768A1214");

// Caratteristica (Read, Write, Notify)
BLEByteCharacteristic switchChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite | BLENotify);

void setup(){
    
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    if (!BLE.begin())
    {
        Serial.println("Errore avvio BLE!");
        while (1)
            ;
    }

    BLE.setLocalName("NanoIoT");
    BLE.setAdvertisedService(actuatorService);
    actuatorService.addCharacteristic(switchChar);
    BLE.addService(actuatorService);

    switchChar.writeValue(0); // Valore iniziale spento

    BLE.advertise();
    Serial.println("Nano IoT in attesa di connessione...");
}

void loop(){

    BLEDevice central = BLE.central();

    if (central)
    {
        Serial.print("Connesso al master: ");
        Serial.println(central.address());

        while (central.connected())
        {
            if (switchChar.written())
            {
                byte val = switchChar.value();
                if (val == 1)
                {
                    digitalWrite(ledPin, HIGH);
                    Serial.println("PIN 4 ACCESO");
                }
                else
                {
                    digitalWrite(ledPin, LOW);
                    Serial.println("PIN 4 SPENTO");
                }
            }
        }
        Serial.println("Disconnesso");
    }
}