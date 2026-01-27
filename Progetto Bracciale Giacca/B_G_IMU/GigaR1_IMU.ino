#include "Imu3DVisualizer.h"
#include <Arduino_GigaDisplay_GFX.h>

// 1. Instantiate the display OBJECT here (global)
GigaDisplay_GFX gigaDisplay;

// 2. Instantiate your visualizer
Imu3DVisualizer visualizer;

void setup()
{
  // Initialize Display first!
  gigaDisplay.begin();

  // Initialize Visualizer
  visualizer.begin();
}

void loop()
{
  visualizer.updateAndDraw();

  // Use a smaller delay for smoother animation
  // The visualizer uses delta-time, so speed will remain correct
  delay(20);
}