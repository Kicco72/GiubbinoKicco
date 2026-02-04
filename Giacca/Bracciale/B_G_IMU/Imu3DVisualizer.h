#ifndef IMU3DVISUALIZER_H
#define IMU3DVISUALIZER_H

#include <Arduino.h>
#include <Arduino_BMI270_BMM150.h>
#include <Arduino_GigaDisplay_GFX.h> 
#include <Wire.h>

// Color Definitions
#define CIANO   0x07FF
#define ROSSO   0xf800
#define BLE     0x001F
#define VERDE   0x07E0
#define BIANCO  0xffff
#define NERO    0x0000
#define GRIGIO  0x3186

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

private:
    // Current orientation
    float pitch = 0.0;
    float roll = 0.0;
    float yaw = 0.0;

    // Timing for physics integration
    unsigned long lastUpdateMicros = 0;

    // Previous coordinates (used for erasing)
    int prevX_end_x = 0, prevX_end_y = 0;
    int prevY_end_x = 0, prevY_end_y = 0;
    int prevZ_end_x = 0, prevZ_end_y = 0;
    bool firstDraw = true;

    Point3D rotatePoint(Point3D p, float pitch, float roll, float yaw);
    void readImuData();
    void drawSphere();
};

#endif // IMU3DVISUALIZER_H