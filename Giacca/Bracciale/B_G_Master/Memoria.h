#ifndef MEMORIA_H
#define MEMORIA_H

#include <Arduino.h>
#include <mbed.h>
#include <Arduino_USBHostMbed5.h>
#include <FATFileSystem.h>
#include <QSPIFBlockDevice.h>
#include <LittleFileSystem.h>
#include <Arduino_GigaDisplay_GFX.h>

class Memoria {
public:
    Memoria();
    bool begin();
    // Salva una riga nel CSV: Data, Ora, Temp, Hum, Press
    bool logData(String date, String time, float temp, float hum, float press);
    void drawContent(GigaDisplay_GFX& display); // Visualizza il contenuto sul display
    void printContent(); // Stampa tutto il file su seriale (per debug)
    void selectDrive(int driveIndex); // 0 = Flash, 1 = USB
    void enterSelectedDrive(); // Entra nella visualizzazione file
    void exitFileList(); // Esce dalla visualizzazione file
    bool isViewingFiles(); // Restituisce true se stiamo guardando la lista file
    int getSelectedDrive(); // Restituisce l'indice del drive selezionato

private:
    // USB
    USBHostMSD* _msd;
    mbed::FATFileSystem* _fsUSB;
    // QSPI (Flash Interna)
    QSPIFBlockDevice* _qspi;
    mbed::LittleFileSystem* _fsQSPI;
    
    bool _mounted;
    int _selectedDrive; // 0 = Flash Interna, 1 = USB Drive
    bool _viewingFiles; // Flag per sapere se siamo nella lista file
    bool ensureConnection(); // Controlla e connette l'USB se necessario
    bool initQSPI(); // Inizializza la memoria interna
    void drawDriveList(GigaDisplay_GFX& display);
    void drawFileList(GigaDisplay_GFX& display);
};

#endif