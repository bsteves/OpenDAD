/******************************************************************************
  OpenDAD
  Main sketch file

  Shared structures, global declarations, setup(), and loop() live here.
******************************************************************************/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

/******************************************************************************
  Shared data structures
******************************************************************************/

struct CalibrationData
{
  // Ten raw sensor readings, corresponding to physical angles 0..90
  // in 10-degree increments.
  float raw[10] = {
    0.0, 10.0, 20.0, 30.0, 40.0,
    50.0, 60.0, 70.0, 80.0, 90.0
  };

  bool has[10] = {
    false, false, false, false, false,
    false, false, false, false, false
  };
};

/******************************************************************************
  Shared global objects and variables
******************************************************************************/

extern CalibrationData cal;
extern Preferences prefs;
extern float targetAngle;
extern bool targetAngleSet;

/******************************************************************************
  Functions provided by Target.ino
******************************************************************************/
void setTargetAngle(float angle);
void setTargetToCurrentAngle();
void clearTargetAngle();
float getTargetDifference();

/******************************************************************************
  Functions provided by Sensor.ino
******************************************************************************/

void setupPins();
uint16_t readSensorRawWord();
float rawWordToDegrees(uint16_t raw);

/******************************************************************************
  Functions provided by Calibration.ino
******************************************************************************/

bool calibrationComplete();
bool calibrationPointSet(float targetAngle);

void loadCalibration();
void saveCalibration();
void resetCalibration();

void setCalibrationPoint(
  float rawAngle,
  float targetAngle
);

float applyCalibration(float rawAngle);
float smoothAngle(float newAngle);

void handleSerialCommands(float rawAngle);

/******************************************************************************
  Functions provided by WiFi.ino
******************************************************************************/

void setupWiFi();
int getConnectedDeviceCount();
bool isDeviceConnected();
void printWiFiStatus();

/******************************************************************************
  Functions provided by WebServer.ino
******************************************************************************/

void setupWebServer();
void handleWebServer();

void updateWebState(
  uint16_t rawWord,
  float rawAngle,
  float displayAngle
);

/******************************************************************************
  Startup information
******************************************************************************/

void printStartupInfo()
{
  Serial.println();
  Serial.println("OpenDAD for Faceting Machines");
  Serial.println("TLE5012 + ESP32 web interface");
  Serial.println();

  Serial.println("Calibration commands (set mast to reference angle first):");
  Serial.println("  0, 10, 20, 30, 40, 50, 60, 70, 80, 90");
  Serial.println("  r = reset calibration");
  Serial.println();
}

/******************************************************************************
  Arduino setup
******************************************************************************/

void setup()
{
  Serial.begin(115200);
  delay(1000);

  prefs.begin("dad", false);
  loadCalibration();

  setupPins();
  printStartupInfo();
  setupWiFi();
  setupWebServer();
}

/******************************************************************************
  Arduino loop
******************************************************************************/

void loop()
{
  handleWebServer();

  uint16_t rawWord = readSensorRawWord();
  float rawAngle = rawWordToDegrees(rawWord);

  handleSerialCommands(rawAngle);

  float calibratedAngle = applyCalibration(rawAngle);
  float displayAngle = smoothAngle(calibratedAngle);

  updateWebState(
    rawWord,
    rawAngle,
    displayAngle
  );

  Serial.printf(
    "RAW: 0x%04X  RAW_ANGLE: %.2f  DISPLAY: %.2f\n",
    rawWord,
    rawAngle,
    displayAngle
  );

  delay(20);
}