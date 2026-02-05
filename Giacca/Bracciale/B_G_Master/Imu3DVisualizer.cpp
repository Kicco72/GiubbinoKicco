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
  // RIMOSSO: gigaDisplay.drawCircle(600, 240, 100, GRIGIO_CHIARO); // Ora disegniamo la sfera dinamica in drawSphere

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
    // MODIFICA: L'orientamento ora è gestito dal giroscopio.
    // L'accelerometro serve solo per il vettore intensità.
    // Commentiamo il calcolo basato su accelerometro:
    // roll = atan2(ay, az) * RAD_TO_DEG;
    // pitch = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;
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
    // Integrazione Giroscopio per orientamento (dps * secondi = gradi)
    // Roll (X), Pitch (Y), Yaw (Z)
    roll += gx * dt;
    pitch += gy * dt;
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

  // 1. Pulisci l'intera area della sfera per il nuovo frame
  // Questo rimuove assi, vettori e la sfera precedente
  gigaDisplay.fillCircle(cx, cy, r + 5, NERO);

  // 2. Disegna Sfera Wireframe (3 Cerchi Ortogonali Rotanti)
  uint16_t sphereColor = GRIGIO_SCURO;
  float step = 15.0; // Risoluzione della sfera

  for (int axis = 0; axis < 3; axis++)
  {
    float prev_x = 0, prev_y = 0;
    bool first = true;
    for (float a = 0; a <= 360; a += step)
    {
      float rad = a * DEG_TO_RAD;
      float c = cos(rad) * r;
      float s = sin(rad) * r;

      Point3D p;
      if (axis == 0)
        p = {0, c, s}; // Piano YZ
      else if (axis == 1)
        p = {c, 0, s}; // Piano XZ
      else
        p = {c, s, 0}; // Piano XY

      // Sfera fissa: Ruotata su asse X di ancora -10 gradi (Pitch -30, Roll -70, Yaw -60)
      Point3D rot = rotatePoint(p, -30, -70, -60);
      int sx = cx + (int)rot.x;
      int sy = cy + (int)rot.y;

      if (!first)
        gigaDisplay.drawLine((int)prev_x, (int)prev_y, sx, sy, sphereColor);
      prev_x = sx;
      prev_y = sy;
      first = false;
    }
  }

  // --- Punti Cardinali Fissi (N = +X) ---
  gigaDisplay.setTextColor(BIANCO);
  gigaDisplay.setTextSize(2);

  Point3D cardinalPoints[] = {
      {(float)r, 0, 0},  // N (+X)
      {-(float)r, 0, 0}, // S (-X)
      {0, (float)r, 0},  // E (+Y)
      {0, -(float)r, 0}  // W (-Y)
  };
  const char *cardinalLabels[] = {"N", "S", "E", "W"};

  for (int i = 0; i < 4; i++)
  {
    // Stessa rotazione isometrica della sfera
    Point3D rot = rotatePoint(cardinalPoints[i], -30, -70, -60);
    int sx = cx + (int)rot.x;
    int sy = cy + (int)rot.y;
    gigaDisplay.setCursor(sx - 6, sy - 6);
    gigaDisplay.print(cardinalLabels[i]);
  }

  // --- 3D Rotation Logic ---
  Point3D xAxis = {(float)r, 0, 0};
  Point3D yAxis = {0, (float)r, 0};
  Point3D zAxis = {0, 0, (float)r};

  // 1. Calcola orientamento assi IMU (Body -> World)
  Point3D xBody = rotatePoint(xAxis, pitch, roll, yaw);
  Point3D yBody = rotatePoint(yAxis, pitch, roll, yaw);
  Point3D zBody = rotatePoint(zAxis, pitch, roll, yaw);

  // 2. Applica la rotazione della vista fissa (World -> Screen)
  // Angoli fissati: Pitch -30, Roll -70, Yaw -60
  Point3D xRot = rotatePoint(xBody, -30, -70, -60);
  Point3D yRot = rotatePoint(yBody, -30, -70, -60);
  Point3D zRot = rotatePoint(zBody, -30, -70, -60);

  // --- Acceleration Vector Logic ---
  // Scaliamo il vettore accelerazione (1g = 100px raggio)
  float accScale = 100.0;
  Point3D accVec = {dbg_ax * accScale, dbg_ay * accScale, dbg_az * accScale};
  
  // Ruota vettore accelerazione (Body -> World -> Screen)
  Point3D accWorld = rotatePoint(accVec, pitch, roll, yaw);
  Point3D accRot = rotatePoint(accWorld, -30, -70, -60);

  int newXx = cx + (int)xRot.x;
  int newXy = cy + (int)xRot.y;
  int newYx = cx + (int)yRot.x;
  int newYy = cy + (int)yRot.y;
  int newZx = cx + (int)zRot.x;
  int newZy = cy + (int)zRot.y;
  int newAccX = cx + (int)accRot.x;
  int newAccY = cy + (int)accRot.y;

  gigaDisplay.setTextSize(2);

  // Draw new lines
  gigaDisplay.drawLine(cx, cy, newXx, newXy, ROSSO);
  gigaDisplay.setTextColor(ROSSO);
  gigaDisplay.setCursor(newXx, newXy);
  gigaDisplay.print("X");

  gigaDisplay.drawLine(cx, cy, newYx, newYy, VERDE);
  gigaDisplay.setTextColor(VERDE);
  gigaDisplay.setCursor(newYx, newYy);
  gigaDisplay.print("Y");

  gigaDisplay.drawLine(cx, cy, newZx, newZy, BLE);
  gigaDisplay.setTextColor(BLE);
  gigaDisplay.setCursor(newZx, newZy);
  gigaDisplay.print("Z");

  // Disegna nuovo vettore accelerazione (Giallo)
  gigaDisplay.drawLine(cx, cy, newAccX, newAccY, GIALLO);

  // --- DEBUG DASHBOARD (Update every 100ms) ---
  static unsigned long lastTxt = 0;
  if (millis() - lastTxt > 100)
  {
    lastTxt = millis();

    // Clear value areas
    // Riduciamo la larghezza per non cancellare la sfera
    // MODIFICA: Aumentata larghezza a 360 per evitare sovrapposizione testo
    gigaDisplay.fillRect(120, 180, 360, 100, NERO);

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