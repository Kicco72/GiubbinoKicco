#include "Bussola.h"
#include <math.h>

#define CENTRO_X 400
#define CENTRO_Y 200
#define RAGGIO 120
#define BIANCO 0xffff
#define NERO 0x0000
#define ROSSO 0xf800

Bussola::Bussola() {}

void Bussola::begin() {}

void Bussola::drawBackground()
{
    // Disegna il quadrante
    gigaDisplay.drawCircle(CENTRO_X, CENTRO_Y, RAGGIO, BIANCO);
    gigaDisplay.drawCircle(CENTRO_X, CENTRO_Y, RAGGIO + 1, BIANCO);

    // Punti cardinali
    gigaDisplay.setTextSize(3);
    gigaDisplay.setTextColor(BIANCO);
    gigaDisplay.setCursor(CENTRO_X - 10, CENTRO_Y - RAGGIO - 40);
    gigaDisplay.print("N");
    gigaDisplay.setCursor(CENTRO_X - 10, CENTRO_Y + RAGGIO + 15);
    gigaDisplay.print("S");
    gigaDisplay.setCursor(CENTRO_X + RAGGIO + 15, CENTRO_Y - 10);
    gigaDisplay.print("E");
    gigaDisplay.setCursor(CENTRO_X - RAGGIO - 40, CENTRO_Y - 10);
    gigaDisplay.print("W");
}

void Bussola::updateAndDraw(float x, float y, float z)
{
    // Calcolo Heading (semplificato) in radianti
    // Nota: La calibrazione e la compensazione tilt (con accelerometro) migliorerebbero la precisione
    // Modifica: Ruotiamo di -90 gradi (PI/2) affinché l'asse X (valore più alto) punti verso l'alto (N)
    float heading = atan2(y, x) - (PI / 2.0);

    // Converti in gradi per visualizzazione numerica
    // Aggiungiamo 90 gradi per compensare la rotazione della lancetta e avere 0° al Nord
    float headingDeg = (heading * 180.0 / PI) + 90.0;
    if (headingDeg < 0)
        headingDeg += 360;

    // Pulisci l'interno della bussola
    gigaDisplay.fillCircle(CENTRO_X, CENTRO_Y, RAGGIO - 5, NERO);

    // Disegna la lancetta che punta al Nord
    // L'angolo atan2(y,x) è la direzione del vettore magnetico.
    // Se il nord è davanti (X), y=0, angle=0.
    drawNeedle(heading, ROSSO);

    // Info Testuali
    gigaDisplay.fillRect(0, 340, 800, 40, NERO); // Pulisci area testo sopra i bottoni (spostato a Y=340)
    gigaDisplay.setCursor(20, 350);
    gigaDisplay.setTextColor(BIANCO);
    gigaDisplay.setTextSize(2);
    gigaDisplay.print("Heading: ");
    gigaDisplay.print(headingDeg, 1);
    gigaDisplay.print(" deg   (X:");
    gigaDisplay.print(x, 0);
    gigaDisplay.print(" Y:");
    gigaDisplay.print(y, 0);
    gigaDisplay.print(")");
}

void Bussola::drawNeedle(float angle, uint16_t color)
{
    // Calcola la punta della lancetta
    int x2 = CENTRO_X + (int)(cos(angle) * (RAGGIO - 20));
    int y2 = CENTRO_Y + (int)(sin(angle) * (RAGGIO - 20));

    // Disegna linea e centro
    gigaDisplay.drawLine(CENTRO_X, CENTRO_Y, x2, y2, color);
    gigaDisplay.fillCircle(CENTRO_X, CENTRO_Y, 5, color);
}