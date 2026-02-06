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

void Imu3DVisualizer::tare()
{
  // 1. Resetta l'orientamento (Integratori) a zero
  pitch = 0;
  roll = 0;
  yaw = 0;
  
  // Resetta gli offset visivi (ora gestiti resettando direttamente le variabili sopra)
  pitchOffset = 0;
  rollOffset = 0;
  yawOffset = 0;

  // 2. Calibra i sensori: Imposta il valore attuale come "Zero" (Bias)
  // Aggiungiamo il valore corrente al bias esistente per permettere tare successive
  if (accSuccess) {
      accBiasX += dbg_ax;
      accBiasY += dbg_ay;
      accBiasZ += dbg_az;
  }
  gyroBiasX += dbg_gx;
  gyroBiasY += dbg_gy;
  gyroBiasZ += dbg_gz;

  // Resetta anche i filtri di smoothing per fermare eventuali inerzie residue
  filt_gx = 0.0;
  filt_gy = 0.0;
  filt_gz = 0.0;
}

void Imu3DVisualizer::drawBackground()
{
  // Draw static reference circle
  // Spostiamo la sfera a destra (X=600) per lasciare spazio al testo a sinistra
  // RIMOSSO: gigaDisplay.drawCircle(600, 240, 100, GRIGIO_CHIARO); // Ora disegniamo la sfera dinamica in drawSphere

  // Draw static labels for Debugging
  gigaDisplay.setTextSize(2);
  gigaDisplay.setTextColor(BIANCO);
  gigaDisplay.setCursor(10, 30);
  gigaDisplay.print("RAW ACC:");
  gigaDisplay.setCursor(10, 70);
  gigaDisplay.print("RPY DEG:");
  gigaDisplay.setCursor(10, 110);
  gigaDisplay.print("INTENSITA': ");
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
    // Applica la calibrazione (Tare)
    ax -= accBiasX;
    ay -= accBiasY;
    az -= accBiasZ;

    accSuccess = true;
    dbg_ax = ax;
    dbg_ay = ay;
    dbg_az = az; // Store for display
  }
  else
  {
    accSuccess = false;
  }

  // Attempt to read Gyroscope
  if (imu.readGyroscope(gx, gy, gz))
  {
    // Applica la calibrazione (Tare)
    gx -= gyroBiasX;
    gy -= gyroBiasY;
    gz -= gyroBiasZ;

    // --- FILTRO LOW PASS (Smoothing) ---
    // Riduce la sensibilità ai movimenti rapidi e il jitter
    float alphaSmooth = 0.1; // 10% nuovo dato, 90% storico (Molto fluido/lento)
    filt_gx = (filt_gx * (1.0 - alphaSmooth)) + (gx * alphaSmooth);
    filt_gy = (filt_gy * (1.0 - alphaSmooth)) + (gy * alphaSmooth);
    filt_gz = (filt_gz * (1.0 - alphaSmooth)) + (gz * alphaSmooth);

    // Usa i valori filtrati per l'integrazione
    gx = filt_gx;
    gy = filt_gy;
    gz = filt_gz;

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
    // Modifica: Asse X = Pitch, Asse Y = Roll, Asse Z = Yaw
    pitch += gx * dt; // X -> Pitch
    roll += gy * dt;  // Y -> Roll
    yaw += gz * dt;

    // --- FILTRO COMPLEMENTARE ---
    // "Calcola il basso": Usiamo l'accelerometro (Gravità) per correggere il drift del giroscopio.
    // L'accelerometro rileva la gravità (circa 9.81 m/s^2 o 1g sull'asse Z a riposo), che indica il "Sotto".
    if (accSuccess)
    {
      // Calcolo angoli da accelerometro secondo la nuova mappatura
      // Pitch (X): Rotazione attorno asse X -> atan2(ay, az)
      float acc_pitch = atan2(ay, az) * RAD_TO_DEG;
      // Roll (Y): Rotazione attorno asse Y -> atan2(-ax, ...)
      float acc_roll = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;

      float alpha = 0.98; // 98% Giroscopio, 2% Accelerometro
      pitch = alpha * pitch + (1.0 - alpha) * acc_pitch;
      roll = alpha * roll + (1.0 - alpha) * acc_roll;
    }
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

  // Calcola gli angoli effettivi sottraendo l'offset di tara
  float p_imu = pitch - pitchOffset;
  float r_imu = roll - rollOffset;
  float y_imu = yaw - yawOffset;

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

      // 1. Applica rotazione IMU (Device -> World)
      Point3D p_world = rotatePoint(p, p_imu, r_imu, y_imu);
      // 2. Applica rotazione Vista Fissa (World -> Screen)
      Point3D rot = rotatePoint(p_world, -30, -70, -60);
      int sx = cx + (int)rot.x;
      int sy = cy + (int)rot.y;

      if (!first)
        gigaDisplay.drawLine((int)prev_x, (int)prev_y, sx, sy, sphereColor);
      prev_x = sx;
      prev_y = sy;
      first = false;
    }
  }

  // --- Asse X (N-S) Rosso ---
  Point3D pN = {(float)r, 0, 0};
  Point3D pS = {-(float)r, 0, 0};

  // Rotazione N
  Point3D pN_w = rotatePoint(pN, p_imu, r_imu, y_imu);
  Point3D pN_rot = rotatePoint(pN_w, -30, -70, -60);
  int nx = cx + (int)pN_rot.x;
  int ny = cy + (int)pN_rot.y;

  // Rotazione S
  Point3D pS_w = rotatePoint(pS, p_imu, r_imu, y_imu);
  Point3D pS_rot = rotatePoint(pS_w, -30, -70, -60);
  int sx_pt = cx + (int)pS_rot.x;
  int sy_pt = cy + (int)pS_rot.y;

  gigaDisplay.drawLine(nx, ny, sx_pt, sy_pt, ROSSO);
  gigaDisplay.setTextColor(ROSSO);
  gigaDisplay.setCursor(nx + 10, ny);
  gigaDisplay.print("x");

  // --- Asse Y (W-E) Verde ---
  Point3D pW = {0, (float)r, 0};
  Point3D pE = {0, -(float)r, 0};

  // Rotazione W
  Point3D pW_w = rotatePoint(pW, p_imu, r_imu, y_imu);
  Point3D pW_rot = rotatePoint(pW_w, -30, -70, -60);
  int wx = cx + (int)pW_rot.x;
  int wy = cy + (int)pW_rot.y;

  // Rotazione E
  Point3D pE_w = rotatePoint(pE, p_imu, r_imu, y_imu);
  Point3D pE_rot = rotatePoint(pE_w, -30, -70, -60);
  int ex = cx + (int)pE_rot.x;
  int ey = cy + (int)pE_rot.y;

  gigaDisplay.drawLine(wx, wy, ex, ey, VERDE);
  gigaDisplay.setTextColor(VERDE);
  gigaDisplay.setCursor(wx + 10, wy);
  gigaDisplay.print("y");

  // --- Asse Z (Alto-Basso) Blu ---
  Point3D pAlto = {0, 0, (float)r};
  Point3D pBasso = {0, 0, -(float)r};

  // Rotazione Alto
  Point3D pAlto_w = rotatePoint(pAlto, p_imu, r_imu, y_imu);
  Point3D pAlto_rot = rotatePoint(pAlto_w, -30, -70, -60);
  int ax = cx + (int)pAlto_rot.x;
  int ay = cy + (int)pAlto_rot.y;

  // Rotazione Basso
  Point3D pBasso_w = rotatePoint(pBasso, p_imu, r_imu, y_imu);
  Point3D pBasso_rot = rotatePoint(pBasso_w, -30, -70, -60);
  int bx = cx + (int)pBasso_rot.x;
  int by = cy + (int)pBasso_rot.y;

  gigaDisplay.drawLine(ax, ay, bx, by, BLE);
  gigaDisplay.setTextColor(BLE);
  gigaDisplay.setCursor(ax + 10, ay);
  gigaDisplay.print("z");

  // --- Punti Cardinali Fissi (N = +X) ---
  gigaDisplay.setTextColor(BIANCO);
  gigaDisplay.setTextSize(2);

  Point3D cardinalPoints[] = {
      {(float)r, 0, 0},  // N (+X)
      {-(float)r, 0, 0}, // S (-X)
      {0, (float)r, 0},  // W (+Y)
      {0, -(float)r, 0}, // E (-Y)
      {0, 0, (float)r},  // Alto (+Z)
      {0, 0, -(float)r}  // Basso (-Z)
  };
  const char *cardinalLabels[] = {"N", "S", "W", "E", "Alto", "Basso"};

  for (int i = 0; i < 6; i++)
  {
    // Stessa rotazione isometrica della sfera
    // 1. Applica rotazione IMU
    Point3D p_world = rotatePoint(cardinalPoints[i], p_imu, r_imu, y_imu);
    // 2. Applica rotazione Vista
    Point3D rot = rotatePoint(p_world, -30, -70, -60);
    int sx = cx + (int)rot.x;
    int sy = cy + (int)rot.y;
    // Centra il testo in base alla lunghezza (circa 12px per carattere con size 2)
    int len = strlen(cardinalLabels[i]);
    gigaDisplay.setCursor(sx - (len * 6), sy - 6);
    gigaDisplay.print(cardinalLabels[i]);
  }

  // --- Acceleration Vector Logic ---
  // Scaliamo il vettore accelerazione (1g = 100px raggio)
  float accScale = 100.0;
  Point3D accVec = {dbg_ax * accScale, dbg_ay * accScale, dbg_az * accScale};
  
  // Ruota vettore accelerazione (Body -> Screen)
  // MODIFICA: Non applichiamo la rotazione IMU per lasciare il vettore "libero"
  // Visualizziamo direttamente le componenti misurate nel sistema di vista
  Point3D accRot = rotatePoint(accVec, -30, -70, -60);

  int newAccX = cx + (int)accRot.x;
  int newAccY = cy + (int)accRot.y;

  gigaDisplay.setTextSize(2);

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
    gigaDisplay.fillRect(120, 30, 360, 120, NERO);

    gigaDisplay.setTextSize(2);

    // ROW 1: RAW ACCELEROMETER
    gigaDisplay.setCursor(120, 30); // Allineato con etichetta RAW ACC
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
    gigaDisplay.setCursor(120, 70); // Allineato con etichetta RPY DEG
    gigaDisplay.setTextColor(CIANO);
    gigaDisplay.print("R:");
    gigaDisplay.print(r_imu, 0);
    gigaDisplay.print(" P:");
    gigaDisplay.print(p_imu, 0);
    gigaDisplay.print(" Y:");
    gigaDisplay.print(y_imu, 0);

    // ROW 3: INTENSITY
    float magnitude = sqrt(dbg_ax * dbg_ax + dbg_ay * dbg_ay + dbg_az * dbg_az);
    gigaDisplay.setCursor(120, 110); // Allineato con etichetta INTENSITA':
    gigaDisplay.setTextColor(GIALLO);
    gigaDisplay.print(magnitude, 2);
    gigaDisplay.print(" g");
  }
}

void Imu3DVisualizer::updateAndDraw()
{
  readImuData();
  drawSphere();
}