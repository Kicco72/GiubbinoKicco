#ifndef BUSSOLA_H
#define BUSSOLA_H

#include <Arduino.h>
#include <Arduino_GigaDisplay_GFX.h>

extern GigaDisplay_GFX gigaDisplay;

class Bussola {
public:
    Bussola();
    void begin();
    void drawBackground();
    void updateAndDraw(float x, float y, float z);

private:
    void drawNeedle(float angle, uint16_t color);
};

#endif