/*
  BleNetwork.cpp
  Implementazione della logica BLE
*/
#include "BleNetwork.h"

BleNetwork::BleNetwork()
{
    _senseConnected = false;
    _iotConnected = false;
    _lastTemperature = 0.0;
    _actuatorState = false;

    // UUIDs definiti negli sketch precedenti
    _uuidSenseService = "181A";
    _uuidSenseCharTemp = "2A6E";
    _uuidIoTService = "19B10000-E8F2-537E-4F6C-D104768A1214";
    _uuidIoTCharSwitch = "19B10001-E8F2-537E-4F6C-D104768A1214";
}

void BleNetwork::begin()
{
    if (!BLE.begin())
    {
        Serial.println("Errore avvio BLE!");
        while (1)
            ;
    }
    Serial.println("BLE Network Avviata. Scansione...");
    BLE.scan();
}

void BleNetwork::update()
{
    // Se manca qualcuno, prova a connetterti
    if (!_senseConnected || !_iotConnected)
    {
        scanAndConnect();
    }

    // Gestione Sense
    if (_senseConnected)
    {
        if (!_senseDevice.connected())
        {
            Serial.println("Sense perso!");
            _senseConnected = false;
            BLE.scan(); // Riavvia scansione
        }
        else
        {
            pollSense();
        }
    }

    // Gestione IoT
    if (_iotConnected)
    {
        if (!_iotDevice.connected())
        {
            Serial.println("IoT perso!");
            _iotConnected = false;
            BLE.scan(); // Riavvia scansione
        }
        // Nota: La scrittura su IoT avviene su richiesta (toggleActuator), non in polling
    }
}

void BleNetwork::scanAndConnect()
{
    BLEDevice peripheral = BLE.available();

    if (peripheral)
    {
        // Trovato Sense?
        if (peripheral.localName() == "NanoSense" && !_senseConnected)
        {
            BLE.stopScan();
            if (connectToSense(peripheral))
            {
                _senseConnected = true;
                Serial.println(">> Sense Connesso");
            }
            BLE.scan(); // Riparti
        }
        // Trovato IoT?
        else if (peripheral.localName() == "NanoIoT" && !_iotConnected)
        {
            BLE.stopScan();
            if (connectToIoT(peripheral))
            {
                _iotConnected = true;
                Serial.println(">> IoT Connesso");
            }
            BLE.scan(); // Riparti
        }
    }
}

bool BleNetwork::connectToSense(BLEDevice p)
{
    if (!p.connect())
        return false;
    if (!p.discoverAttributes())
    {
        p.disconnect();
        return false;
    }
    _senseDevice = p;
    return true;
}

bool BleNetwork::connectToIoT(BLEDevice p)
{
    if (!p.connect())
        return false;
    if (!p.discoverAttributes())
    {
        p.disconnect();
        return false;
    }
    _iotDevice = p;
    return true;
}

void BleNetwork::pollSense()
{
    BLECharacteristic tChar = _senseDevice.characteristic(_uuidSenseCharTemp);

    if (tChar && tChar.valueUpdated())
    {
        float temp = 0.0;
        // CORREZIONE PER GIGA R1 (Puntatori)
        tChar.readValue(&temp, sizeof(temp));
        _lastTemperature = temp;
        Serial.print("Temp aggiornata: ");
        Serial.println(_lastTemperature);
    }
}

void BleNetwork::writeIoT(bool state)
{
    if (!_iotConnected)
        return;

    BLECharacteristic sChar = _iotDevice.characteristic(_uuidIoTCharSwitch);
    if (sChar)
    {
        byte val = state ? 1 : 0;
        sChar.writeValue(&val, 1); // Invio sicuro
    }
}

// --- Metodi Pubblici ---

float BleNetwork::getLatestTemperature()
{
    return _lastTemperature;
}

void BleNetwork::toggleActuator()
{
    _actuatorState = !_actuatorState;
    writeIoT(_actuatorState);
    Serial.print("Comando inviato: ");
    Serial.println(_actuatorState);
}

bool BleNetwork::isSenseConnected() { return _senseConnected; }
bool BleNetwork::isIoTConnected() { return _iotConnected; }