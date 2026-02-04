#include "BleNetwork.h"

// --- Costruttore ---
BleNetwork::BleNetwork()
{
    // Inizializzazione stati
    _senseConnected = false;
    _iotConnected = false;
    _lastTemperature = 0.0;
    _actuatorState = false;
    
    // Inizializziamo la nostra flag di scansione a false
    _isScanning = false; 

    // Definizione degli UUID
    _uuidSenseService = "181A";      
    _uuidSenseCharTemp = "2A6E";     
    
    _uuidIoTService = "19B10000-E8F2-537E-4F6C-D104768A1214";
    _uuidIoTCharSwitch = "19B10001-E8F2-537E-4F6C-D104768A1214";
}

// --- Inizializzazione ---
void BleNetwork::begin()
{
    if (!BLE.begin())
    {
        Serial.println("ERRORE: Impossibile avviare il modulo BLE!");
        while (1);
    }
    Serial.println("Stato: Modulo BLE avviato correttamente.");
}

// --- Gestione Scansione ---

void BleNetwork::startScan() {
    // Controllo la mia variabile interna invece di BLE.isScanning()
    if (!_isScanning) {
        Serial.println("Comando: Avvio scansione manuale...");
        BLE.scan(true); 
        _isScanning = true; // Aggiorno lo stato
    }
}

void BleNetwork::stopScan() {
    // Controllo la mia variabile interna
    if (_isScanning) {
        BLE.stopScan();
        Serial.println("Comando: Scansione interrotta.");
        _isScanning = false; // Aggiorno lo stato
    }
}

// Getter pubblico per sapere se stiamo scansionando
bool BleNetwork::isScanning() {
    return _isScanning; // Ritorno la variabile locale, NON chiamo la libreria
}

// --- Loop Principale (Update) ---
void BleNetwork::update()
{
    // 1. Uso la variabile interna per sapere se devo cercare dispositivi
    if (_isScanning) {
        scanAndConnect();
    }

    // 2. Gestione Dispositivo Sense
    if (_senseConnected)
    {
        if (!_senseDevice.connected())
        {
            Serial.println("Avviso: Connessione con Sense persa!");
            _senseConnected = false;
        }
        else
        {
            pollSense();
        }
    }

    // 3. Gestione Dispositivo IoT
    if (_iotConnected)
    {
        if (!_iotDevice.connected())
        {
            Serial.println("Avviso: Connessione con IoT persa!");
            _iotConnected = false;
        }
    }
}

// --- Logica di Connessione ---
void BleNetwork::scanAndConnect()
{
    BLEDevice peripheral = BLE.available();

    if (peripheral)
    {
        // Caso A: NanoSense
        if (peripheral.localName() == "NanoSense" && !_senseConnected)
        {
            // Importante: usare il metodo della classe che gestisce anche la flag _isScanning
            stopScan(); 
            
            if (connectToSense(peripheral))
            {
                _senseConnected = true;
                Serial.println(">> Successo: Dispositivo Sense Connesso");
            }
            else 
            {
                 Serial.println(">> Errore: Connessione a Sense fallita");
            }
            
            // Riavvia usando il metodo della classe
            startScan(); 
        }
        // Caso B: NanoIoT
        else if (peripheral.localName() == "NanoIoT" && !_iotConnected)
        {
            stopScan(); // Usa il metodo interno
            
            if (connectToIoT(peripheral))
            {
                _iotConnected = true;
                Serial.println(">> Successo: Dispositivo IoT Connesso");
            }
            else 
            {
                 Serial.println(">> Errore: Connessione a IoT fallita");
            }
            
            startScan(); // Usa il metodo interno
        }
    }
}

// Helper per connettere Sense
bool BleNetwork::connectToSense(BLEDevice p)
{
    if (!p.connect()) {
        return false;
    }
    if (!p.discoverAttributes())
    {
        p.disconnect();
        return false;
    }
    _senseDevice = p;
    return true;
}

// Helper per connettere IoT
bool BleNetwork::connectToIoT(BLEDevice p)
{
    if (!p.connect()) {
        return false;
    }
    if (!p.discoverAttributes())
    {
        p.disconnect();
        return false;
    }
    _iotDevice = p;
    return true;
}

// --- Lettura e Scrittura Dati ---

void BleNetwork::pollSense()
{
    BLECharacteristic tChar = _senseDevice.characteristic(_uuidSenseCharTemp);

    if (tChar && tChar.valueUpdated())
    {
        float temp = 0.0;
        tChar.readValue(&temp, sizeof(temp));
        
        _lastTemperature = temp;
        Serial.print("Dati: Temperatura aggiornata -> ");
        Serial.print(_lastTemperature);
        Serial.println(" °C");
    }
}

void BleNetwork::writeIoT(bool state)
{
    if (!_iotConnected) {
        Serial.println("Errore: Impossibile scrivere, IoT non connesso.");
        return;
    }

    BLECharacteristic sChar = _iotDevice.characteristic(_uuidIoTCharSwitch);
    if (sChar)
    {
        byte val = state ? 1 : 0;
        sChar.writeValue(&val, 1); 
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
    
    Serial.print("Azione: Comando interruttore inviato -> ");
    Serial.println(_actuatorState ? "ON" : "OFF");
}

bool BleNetwork::isSenseConnected() { return _senseConnected; }
bool BleNetwork::isIoTConnected() { return _iotConnected; }