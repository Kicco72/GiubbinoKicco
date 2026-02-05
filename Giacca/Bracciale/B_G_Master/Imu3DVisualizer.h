#ifndef IMU3DVISUALIZER_H
#define IMU3DVISUALIZER_H

#include <Arduino.h>
#include <Arduino_BMI270_BMM150.h>
#include <Arduino_GigaDisplay_GFX.h> 
#include <Wire.h>

// Definizione dei colori
#define CIANO 0x07FF
#define ROSSO 0xf800
#define BLE 0x001F
#define VERDE 0x07E0
#define MAGENTA 0xF81F
#define BIANCO 0xffff
#define NERO 0x0000
#define GIALLO 0xFFE0
#define GRIGIO_CHIARO 0x6666
#define GRIGIO_SCURO 0x4444



// Reference to the display object defined in the main sketch
extern GigaDisplay_GFX gigaDisplay; 

struct Point3D {
    float x;
    float y;
    float z;
};

class Imu3DVisualizer {
public:
    Imu3DVisualizer();
    bool begin();
    void updateAndDraw();
    void drawBackground(); // Nuovo metodo per disegnare gli elementi statici

private:
    // Current orientation
    float pitch = 0.0;
    float roll = 0.0;
    float yaw = 0.0;

    // Timing for physics integration
    unsigned long lastUpdateMicros = 0;

    Point3D rotatePoint(Point3D p, float pitch, float roll, float yaw);
    void readImuData();
    void drawSphere();
};

#endif // IMU3DVISUALIZER_H