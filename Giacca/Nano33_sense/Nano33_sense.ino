/*
  Nano33_sense.ino
  Progetto Bracciale - Periferica Sense
  Codice per Arduino Nano 33 BLE Sense
  Funzione: Periferica BLE che invia la temperatura al Master
*/

#include <ArduinoBLE.h>
#include <Arduino_HS300x.h>        // Libreria per sensore temperatura (Rev1). Usa Arduino_HS300x per Rev2.
#include <Arduino_LPS22HB.h>       // Libreria per sensore pressione
#include <Arduino_BMI270_BMM150.h> // Libreria per IMU/Magnetometro (Rev2)

// UUID del servizio Environmental Sensing (Standard 0x181A)
BLEService envService("181A");

// UUID della caratteristica Temperature (Standard 0x2A6E)
// Nota: Il Master si aspetta un float (4 byte), quindi usiamo BLEFloatCharacteristic
BLEFloatCharacteristic tempCharacteristic("2A6E", BLERead | BLENotify);
// UUID della caratteristica Humidity (Standard 0x2A6F)
BLEFloatCharacteristic humCharacteristic("2A6F", BLERead | BLENotify);
// UUID della caratteristica Pressure (Standard 0x2A6D)
BLEFloatCharacteristic pressCharacteristic("2A6D", BLERead | BLENotify);
// UUID per Magnetometro (Custom o Standard 2AA1 Magnetic Flux Density 3D)
// Usiamo una caratteristica generica di 12 byte (3 float x 4 byte)
BLECharacteristic magCharacteristic("2AA1", BLERead | BLENotify, 12);

long previousEnvMillis = 0; // Timer per dati ambientali (lenti)
long previousMagMillis = 0; // Timer per magnetometro (veloce)

void setup()
{
  Serial.begin(115200);

  // Inizializzazione sensore HTS221
  if (!HS300x.begin())
  {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1)
      ;
  }

  // Inizializzazione sensore LPS22HB
  if (!BARO.begin())
  {
    Serial.println("Errore: Impossibile inizializzare sensore LPS22HB!");
    while (1)
      ;
  }

  // Inizializzazione IMU (Magnetometro)
  if (!IMU.begin())
  {
    Serial.println("Errore: Impossibile inizializzare IMU!");
    while (1)
      ;
  }

  // Inizializzazione BLE
  if (!BLE.begin())
  {
    Serial.println("Errore: Impossibile avviare BLE!");
    while (1)
      ;
  }

  // Imposta il nome locale che il Master cerca ("NanoSense")
  BLE.setLocalName("NanoSense");

  // Imposta il servizio pubblicizzato
  BLE.setAdvertisedService(envService);

  // Aggiungi la caratteristica al servizio
  envService.addCharacteristic(tempCharacteristic);
  envService.addCharacteristic(humCharacteristic);
  envService.addCharacteristic(pressCharacteristic);
  envService.addCharacteristic(magCharacteristic);

  // Aggiungi il servizio
  BLE.addService(envService);

  // Avvia la pubblicità
  BLE.advertise();

  Serial.println("NanoSense pronto e in ascolto...");
}

void loop()
{
  // Attendi connessione da un centrale
  BLEDevice central = BLE.central();

  if (central)
  {
    Serial.print("Connesso al centrale: ");
    Serial.println(central.address());

    while (central.connected())
    {
      long currentMillis = millis();

      // 1. Dati Ambientali: Aggiornamento lento (ogni 2 secondi)
      if (currentMillis - previousEnvMillis >= 2000)
      {
        previousEnvMillis = currentMillis;

        float temperature = HS300x.readTemperature(); // °C
        float humidity = HS300x.readHumidity();       // %
        float pressure = BARO.readPressure();

        Serial.print("Temperatura inviata: ");
        Serial.print(temperature);
        Serial.println(" °C");

        Serial.print("Umidità inviata: ");
        Serial.print(humidity);
        Serial.println(" %");

        Serial.print("Pressione inviata: ");
        Serial.print(pressure);
        Serial.println(" kPa");

        tempCharacteristic.writeValue(temperature);
        humCharacteristic.writeValue(humidity);
        pressCharacteristic.writeValue(pressure);
      }

      // 2. Magnetometro: Aggiornamento veloce (ogni 100ms = 10Hz)
      if (currentMillis - previousMagMillis >= 100)
      {
        previousMagMillis = currentMillis;

        float x, y, z;
        IMU.readMagneticField(x, y, z);

        // Invia array di 3 float
        float magData[3];
        magData[0] = x;
        magData[1] = y;
        magData[2] = z;
        magCharacteristic.writeValue((byte *)magData, 12);
      }
    }

    Serial.println("Disconnesso dal centrale.");
  }
}