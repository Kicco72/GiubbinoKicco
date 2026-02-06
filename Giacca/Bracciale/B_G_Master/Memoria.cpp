#include "Memoria.h"

Memoria::Memoria() : _msd(nullptr), _fsUSB(nullptr), _qspi(nullptr), _fsQSPI(nullptr), _mounted(false), _selectedDrive(0) {}

bool Memoria::begin() {
    // Istanzia il driver per USB Mass Storage e il FileSystem FAT
    if (!_msd) _msd = new USBHostMSD();
    if (!_fsUSB) _fsUSB = new mbed::FATFileSystem("usb");

    // Inizializza anche la memoria interna QSPI
    initQSPI();

    Serial.println("Memoria (USB): Inizializzata. In attesa di connessione...");
    // Non blocchiamo qui. La connessione avverrà al primo utilizzo o in ensureConnection().
    return true;
}

bool Memoria::initQSPI() {
    if (!_qspi) _qspi = new QSPIFBlockDevice();
    if (!_fsQSPI) _fsQSPI = new mbed::LittleFileSystem("fs");
    
    if (_qspi->init() != 0) {
        Serial.println("Memoria (QSPI): Errore Init Hardware");
        return false;
    }
    
    int err = _fsQSPI->mount(_qspi);
    if (err) {
        Serial.println("Memoria (QSPI): Mount fallito, formattazione...");
        err = _fsQSPI->reformat(_qspi);
    }
    
    if (err == 0) Serial.println("Memoria (QSPI): Montata correttamente su /fs/");
    return (err == 0);
}

bool Memoria::ensureConnection() {
    // Se già connesso, ritorna true
    if (_msd->connected()) {
        return true;
    }

    // Prova a connettere (non bloccante se il dispositivo non c'è, ma necessario per rilevarlo)
    // Nota: connect() tenta di enumerare il dispositivo
    if (_msd->connect()) {
        Serial.println("Memoria (USB): Dispositivo rilevato!");
        
        // Monta il filesystem
        int err = _fsUSB->mount(_msd);
        if (err) {
            Serial.print("Memoria (USB): Errore mount: ");
            Serial.println(err);
            return false;
        }
        _mounted = true;
        return true;
    }
    return false;
}

bool Memoria::logData(String date, String time, float temp, float hum, float press) {
    if (!ensureConnection()) return false;

    // Apre il file in modalità "append" (aggiungi in coda)
    // Il percorso è /usb/Archivio.csv (dove "usb" è il nome dato al FATFileSystem)
    FILE *f = fopen("/usb/Archivio.csv", "a+");
    if (!f) {
        Serial.println("Memoria: Impossibile aprire il file su USB.");
        return false;
    }

    // Controlla se il file è nuovo (dimensione 0) per scrivere l'intestazione
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size == 0) {
        fprintf(f, "Data,Ora,Temperatura,Umidita,Pressione\n");
    }

    // Scrive la riga di dati
    // fprintf usa le stringhe C, quindi convertiamo le String Arduino con .c_str()
    fprintf(f, "%s,%s,%.2f,%.2f,%.2f\n", date.c_str(), time.c_str(), temp, hum, press);
    
    // Chiude il file per salvare le modifiche
    fclose(f);
    Serial.println("Memoria: Dati salvati su USB.");
    return true;
}

void Memoria::selectDrive(int driveIndex) {
    if (driveIndex >= 0 && driveIndex <= 1) {
        _selectedDrive = driveIndex;
    }
}

void Memoria::enterSelectedDrive() {
    _viewingFiles = true;
}

void Memoria::exitFileList() {
    _viewingFiles = false;
}

bool Memoria::isViewingFiles() {
    return _viewingFiles;
}

int Memoria::getSelectedDrive() {
    return _selectedDrive;
}

void Memoria::drawContent(GigaDisplay_GFX& display) {
    if (_viewingFiles) {
        drawFileList(display);
    } else {
        drawDriveList(display);
    }
}

void Memoria::drawDriveList(GigaDisplay_GFX& display) {
    // Pulisci l'area dei contenuti (lasciando intatti i pulsanti in basso)
    display.fillRect(0, 0, 800, 320, 0x0000); // NERO

    display.setCursor(20, 60);
    display.setTextSize(3); // Aumentata grandezza testo
    display.setTextColor(0xffff, 0x0000); // BIANCO su sfondo NERO
    
    display.println("Esplora Risorse:");
    display.println("--------------------");
    display.println();

    int y = 130; // Coordinata Y iniziale per le voci

    // --- DRIVE 1: FLASH INTERNA (QSPI) ---
    // Seleziona colore in base alla selezione
    if (_selectedDrive == 0) display.setTextColor(0xFFE0, 0x0000); // GIALLO (Selezionato)
    else display.setTextColor(0x07E0, 0x0000); // VERDE (Non selezionato)

    display.setCursor(20, y);
    display.print(_selectedDrive == 0 ? "> " : "  "); // Cursore
    display.print("Flash Interna");
    
    // Info stato (statvfs rimosso per compatibilità)
    display.setTextSize(2);
    display.setCursor(400, y + 5); // Allineamento verticale manuale
    display.println(" (Pronto)");

    display.setTextSize(3); // Ripristina size
    
    y += 60; // Spazio per la prossima voce

    // --- DRIVE 2: USB DRIVE ---
    bool usbConn = ensureConnection();
    
    if (_selectedDrive == 1) display.setTextColor(0xFFE0, 0x0000); // GIALLO
    else if (usbConn) display.setTextColor(0x07E0, 0x0000); // VERDE
    else display.setTextColor(0x6666, 0x0000); // GRIGIO

    display.setCursor(20, y);
    display.print(_selectedDrive == 1 ? "> " : "  ");
    display.print("USB Drive");

    display.setTextSize(2);
    display.setCursor(400, y + 5);
    if (usbConn) {
        display.println(" Connesso");
    } else {
        display.println(" Non rilevato");
    }
    
    // Footer
    display.setTextColor(0xffff, 0x0000);
    display.setTextSize(2);
    display.setCursor(20, 280); // Spostato in alto per non coprire i pulsanti
    display.println("Seleziona un drive per");
    display.println("visualizzare i file.");
}

void Memoria::drawFileList(GigaDisplay_GFX& display) {
    // Pulisci l'area dei contenuti
    display.fillRect(0, 0, 800, 320, 0x0000); // NERO

    display.setCursor(20, 20);
    display.setTextSize(3);
    display.setTextColor(0xffff, 0x0000);
    
    String title = (_selectedDrive == 0) ? "File Flash Interna:" : "File USB Drive:";
    
    display.println(title);
    
    // Intestazione colonne
    display.setTextSize(2);
    display.setTextColor(0xAAAA, 0x0000); // Grigio chiaro
    display.setCursor(20, 60); display.print("Nome");
    display.setCursor(350, 60); display.print("Dimensione");
    display.setCursor(550, 60); display.print("Data");
    display.drawLine(20, 80, 780, 80, 0xffff);

    // Seleziona il filesystem corretto
    mbed::FileSystem* fs = (_selectedDrive == 0) ? (mbed::FileSystem*)_fsQSPI : (mbed::FileSystem*)_fsUSB;
    
    if (_selectedDrive == 1 && !ensureConnection()) {
        display.setTextColor(0xf800, 0x0000); // ROSSO
        display.println("Drive non connesso!");
        return;
    }

    mbed::Dir dir;
    if (dir.open(fs, "/") != 0) {
        display.setTextColor(0xf800, 0x0000);
        display.println("Errore apertura directory.");
        return;
    }

    display.setTextColor(0x07E0, 0x0000); // VERDE
    display.setTextSize(2);

    struct dirent d;
    int lines = 0;
    int y = 100;

    while (dir.read(&d) > 0 && lines < 7) {
        if (d.d_name[0] == '.') continue;
        
        // Nome File (Troncato)
        String fname = d.d_name;
        if (fname.length() > 25) fname = fname.substring(0, 22) + "...";

        display.setCursor(20, y);
        display.print(fname);
        
        // Recupera dimensione
        struct stat st;
        // Usa il nome file relativo alla root del filesystem
        if (fs->stat(d.d_name, &st) == 0) {
            display.setCursor(350, y);
            if (st.st_size < 1024) {
                display.print(st.st_size); display.print(" B");
            } else if (st.st_size < 1024 * 1024) {
                display.print(st.st_size / 1024.0, 1); display.print(" KB");
            } else {
                display.print(st.st_size / 1024.0 / 1024.0, 1); display.print(" MB");
            }

            // Data
            if (st.st_mtime > 0) {
                struct tm *t = localtime(&st.st_mtime);
                if (t) {
                    char dateBuf[24];
                    int year = t->tm_year + 1900;

                    // Controllo validità anno esteso (es. 1970-2099) per includere date di default
                    if (year >= 1970 && year <= 2099) {
                        sprintf(dateBuf, "%02d/%02d/%04d %02d:%02d", 
                                t->tm_mday, 
                                t->tm_mon + 1, 
                                year, 
                                t->tm_hour, 
                                t->tm_min);
                    } else {
                        sprintf(dateBuf, "--/--/---- --:--");
                    }
                    
                    display.setCursor(550, y);
                    display.print(dateBuf);
                }
            }
        }
        
        y += 30;
        lines++;
    }
    dir.close();
    
    if (lines == 0) {
        display.setCursor(20, 100);
        display.println("(Cartella vuota)");
    }
}

void Memoria::printContent() {
    if (!ensureConnection()) return;

    FILE *f = fopen("/usb/Archivio.csv", "r");
    if (!f) {
        Serial.println("Memoria: File Archivio.csv non trovato su USB.");
        return;
    }

    Serial.println("--- INIZIO ARCHIVIO MEMORIA ---");
    char buffer[128];
    // Legge riga per riga
    while (fgets(buffer, sizeof(buffer), f)) {
        Serial.print(buffer);
    }
    Serial.println("--- FINE ARCHIVIO MEMORIA ---");
    fclose(f);
}