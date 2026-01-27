

/*
  NANO 33 BLE SENSE - PERIPHERAL
  Trasmette Temperatura e Umidità
*/
#include <ArduinoBLE.h>
// #include <Arduino_HTS221.h> // Usa Arduino_HS300x se hai la Rev2
#include <Arduino_HS300x.h>

// Definizione Servizio (Environmental Sensing)
BLEService envService("181A");

// Caratteristiche (Standard UUIDs)
BLEFloatCharacteristic tempChar("2A6E", BLERead | BLENotify);
BLEFloatCharacteristic humChar("2A6F", BLERead | BLENotify);

long previousMillis = 0;

void setup()
{
    Serial.begin(9600);

    if (!HS300x.begin())
    {
        Serial.println("Failed to initialize humidity temperature sensor!");
        while (1)
            ;
    }

    if (!BLE.begin())
    {
        Serial.println("Errore avvio BLE!");
        while (1)
            ;
    }

    BLE.setLocalName("NanoSense");
    BLE.setAdvertisedService(envService);

    envService.addCharacteristic(tempChar);
    envService.addCharacteristic(humChar);

    BLE.addService(envService);

    BLE.advertise();
    Serial.println("Nano Sense in attesa di connessione...");
}

void loop()
{
    BLEDevice central = BLE.central();

    if (central)
    {
        Serial.print("Connesso al master: ");
        Serial.println(central.address());

        while (central.connected())
        {
            long currentMillis = millis();
            // Legge e invia ogni 2 secondi
            if (currentMillis - previousMillis >= 2000)
            {
                previousMillis = currentMillis;

                float temperature = HS300x.readTemperature(); // °C
                float humidity = HS300x.readHumidity();       // %

                // Arrotonda a 2 decimali e invia
                tempChar.writeValue(temperature);
                humChar.writeValue(humidity);

                Serial.print("Temp: ");
                Serial.print(temperature);
                Serial.print(" | Hum: ");
                Serial.println(humidity);
            }
        }
        Serial.println("Disconnesso");
    }
}