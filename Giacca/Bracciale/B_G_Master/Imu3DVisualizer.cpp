#include "Imu3DVisualizer.h"

// Instantiate the IMU on Wire1 (Correct for GIGA Display Shield)
BoschSensorClass imu(Wire1);

Imu3DVisualizer::Imu3DVisualizer() {}

bool Imu3DVisualizer::begin()
{
  Serial.begin(115200);
  delay(500);

  // 1. Initialize Display
  gigaDisplay.fillScreen(NERO);
  gigaDisplay.setCursor(10, 20);
  gigaDisplay.setTextSize(3);
  gigaDisplay.setTextColor(BIANCO);
  gigaDisplay.println("IMU Init...");

  // 3. Start IMU
  if (!imu.begin())
  {
    Serial.println("ERROR: IMU begin() failed on Wire1!");
    gigaDisplay.setTextColor(ROSSO);
    gigaDisplay.setCursor(10, 60);
    gigaDisplay.println("IMU FAIL!");
    return false;
  }

  Serial.println("IMU initialized successfully on Wire1!");
  gigaDisplay.setTextColor(VERDE);
  gigaDisplay.setCursor(10, 60);
  gigaDisplay.println("IMU OK!");
  delay(1000);

  lastUpdateMicros = micros();

  // Nota: Non disegniamo più l'UI qui perché verrà gestita dal Master
  // quando si passa alla modalità IMU.
  return true;
}

// Global variables for debug display (to persist between frames)
float dbg_ax = -999, dbg_ay = -999, dbg_az = -999;
float dbg_gx = -999, dbg_gy = -999, dbg_gz = -999;
bool accSuccess = false;

void Imu3DVisualizer::drawBackground()
{
  // Draw static reference circle
  // Spostiamo la sfera a destra (X=600) per lasciare spazio al testo a sinistra
  gigaDisplay.drawCircle(600, 240, 100, GRIGIO_CHIARO);

  // Draw static labels for Debugging
  gigaDisplay.setTextSize(2);
  gigaDisplay.setTextColor(BIANCO);
  gigaDisplay.setCursor(10, 180);
  gigaDisplay.print("RAW ACC:");
  gigaDisplay.setCursor(10, 240);
  gigaDisplay.print("RPY DEG:");
}

void Imu3DVisualizer::readImuData()
{
  float ax, ay, az;
  float gx, gy, gz;

  // --- FORCE READ (Bypass available() check for debugging) ---
  // If these return false, the variables ax/ay/az remain untouched.
  // We initialize them to 0 to ensure we aren't using garbage memory.
  ax = 0;
  ay = 0;
  az = 0;

  // Attempt to read Accelerometer
  // The library returns 1 (true) on success, 0 (false) on failure
  if (imu.readAcceleration(ax, ay, az))
  {
    accSuccess = true;
    dbg_ax = ax;
    dbg_ay = ay;
    dbg_az = az; // Store for display

    // Calculate Pitch and Roll
    // We use -9.81 reference or simply 1g.
    // If the device is flat, z should be ~1.0 or ~0.98 depending on units.
    roll = atan2(ay, az) * RAD_TO_DEG;
    pitch = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;
  }
  else
  {
    accSuccess = false;
  }

  // Attempt to read Gyroscope
  if (imu.readGyroscope(gx, gy, gz))
  {
    dbg_gx = gx;
    dbg_gy = gy;
    dbg_gz = gz; // Store for display

    // Calculate Delta Time safely
    unsigned long currentMicros = micros();
    // Force float division
    float dt = (float)(currentMicros - lastUpdateMicros) / 1000000.0f;
    lastUpdateMicros = currentMicros;

    // Simple integration without deadzone to see if raw values exist
    // If gz is constantly 0.00, we have a sensor config issue.
    yaw += gz * dt;
  }
}

Point3D Imu3DVisualizer::rotatePoint(Point3D p, float pitchDeg, float rollDeg, float yawDeg)
{
  float pRad = pitchDeg * DEG_TO_RAD;
  float rRad = rollDeg * DEG_TO_RAD;
  float yRad = yawDeg * DEG_TO_RAD;

  // X Rotation
  float y1 = p.y * cos(pRad) - p.z * sin(pRad);
  float z1 = p.y * sin(pRad) + p.z * cos(pRad);
  float x1 = p.x;

  // Y Rotation
  float x2 = x1 * cos(rRad) + z1 * sin(rRad);
  float z2 = -x1 * sin(rRad) + z1 * cos(rRad);
  float y2 = y1;

  // Z Rotation
  float x3 = x2 * cos(yRad) - y2 * sin(yRad);
  float y3 = x2 * sin(yRad) + y2 * cos(yRad);
  float z3 = z2;

  return {x3, y3, z3};
}

void Imu3DVisualizer::drawSphere()
{
  int cx = 600; // Spostato a destra
  int cy = 240; // Centro verticale (480/2)
  int r = 100;

  // --- 3D Rotation Logic ---
  Point3D xAxis = {(float)r, 0, 0};
  Point3D yAxis = {0, (float)r, 0};
  Point3D zAxis = {0, 0, (float)r};

  Point3D xRot = rotatePoint(xAxis, pitch, roll, yaw);
  Point3D yRot = rotatePoint(yAxis, pitch, roll, yaw);
  Point3D zRot = rotatePoint(zAxis, pitch, roll, yaw);

  int newXx = cx + (int)xRot.x;
  int newXy = cy + (int)xRot.y;
  int newYx = cx + (int)yRot.x;
  int newYy = cy + (int)yRot.y;
  int newZx = cx + (int)zRot.x;
  int newZy = cy + (int)zRot.y;

  // Erase old lines
  if (!firstDraw)
  {
    gigaDisplay.drawLine(cx, cy, prevX_end_x, prevX_end_y, NERO);
    gigaDisplay.drawLine(cx, cy, prevY_end_x, prevY_end_y, NERO);
    gigaDisplay.drawLine(cx, cy, prevZ_end_x, prevZ_end_y, NERO);
  }

  // Draw new lines
  gigaDisplay.drawLine(cx, cy, newXx, newXy, ROSSO);
  gigaDisplay.drawLine(cx, cy, newYx, newYy, VERDE);
  gigaDisplay.drawLine(cx, cy, newZx, newZy, BLE);

  prevX_end_x = newXx;
  prevX_end_y = newXy;
  prevY_end_x = newYx;
  prevY_end_y = newYy;
  prevZ_end_x = newZx;
  prevZ_end_y = newZy;
  firstDraw = false;

  // --- DEBUG DASHBOARD (Update every 100ms) ---
  static unsigned long lastTxt = 0;
  if (millis() - lastTxt > 100)
  {
    lastTxt = millis();

    // Clear value areas
    // Riduciamo la larghezza per non cancellare la sfera
    gigaDisplay.fillRect(120, 180, 250, 100, NERO);

    gigaDisplay.setTextSize(2);

    // ROW 1: RAW ACCELEROMETER
    gigaDisplay.setCursor(120, 180); // Allineato con etichetta RAW ACC
    if (accSuccess)
      gigaDisplay.setTextColor(BIANCO);
    else
      gigaDisplay.setTextColor(ROSSO); // Red if read failed

    gigaDisplay.print("X:");
    gigaDisplay.print(dbg_ax, 2);
    gigaDisplay.print(" Y:");
    gigaDisplay.print(dbg_ay, 2);
    gigaDisplay.print(" Z:");
    gigaDisplay.print(dbg_az, 2);

    // ROW 2: CALCULATED ANGLES
    gigaDisplay.setCursor(120, 240); // Allineato con etichetta RPY DEG
    gigaDisplay.setTextColor(CIANO);
    gigaDisplay.print("R:");
    gigaDisplay.print(roll, 0);
    gigaDisplay.print(" P:");
    gigaDisplay.print(pitch, 0);
    gigaDisplay.print(" Y:");
    gigaDisplay.print(yaw, 0);
  }
}

void Imu3DVisualizer::updateAndDraw()
{
  readImuData();
  drawSphere();
}