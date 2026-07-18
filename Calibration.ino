/******************************************************************************
  OpenDAD calibration, smoothing, storage, and serial commands
******************************************************************************/

/******************************************************************************
  Global calibration and Preferences objects

  These are declared with extern in OpenDAD.ino and defined here exactly once.
******************************************************************************/

Preferences prefs;
CalibrationData cal;

float smoothedAngle = 0.0;
bool filterInitialized = false;

const float SMOOTHING_ALPHA = 0.3;

/******************************************************************************
  Angle utility functions
******************************************************************************/

float normalizeSigned(float angle)
{
  while (angle > 180.0)
  {
    angle -= 360.0;
  }

  while (angle <= -180.0)
  {
    angle += 360.0;
  }

  return angle;
}

float signedDelta(float fromAngle, float toAngle)
{
  return normalizeSigned(toAngle - fromAngle);
}

/******************************************************************************
  Calibration storage
******************************************************************************/

void saveCalibration()
{
  prefs.putFloat("cal0", cal.raw0);
  prefs.putFloat("cal45", cal.raw45);
  prefs.putFloat("cal90", cal.raw90);

  prefs.putBool("has0", cal.has0);
  prefs.putBool("has45", cal.has45);
  prefs.putBool("has90", cal.has90);
}

void loadCalibration()
{
  cal.raw0 = prefs.getFloat("cal0", 0.0);
  cal.raw45 = prefs.getFloat("cal45", 45.0);
  cal.raw90 = prefs.getFloat("cal90", 90.0);

  cal.has0 = prefs.getBool("has0", false);
  cal.has45 = prefs.getBool("has45", false);
  cal.has90 = prefs.getBool("has90", false);

  Serial.println("Calibration loaded");

  Serial.printf(
    "  0:  %s  raw %.2f\n",
    cal.has0 ? "set" : "not set",
    cal.raw0
  );

  Serial.printf(
    "  45: %s  raw %.2f\n",
    cal.has45 ? "set" : "not set",
    cal.raw45
  );

  Serial.printf(
    "  90: %s  raw %.2f\n",
    cal.has90 ? "set" : "not set",
    cal.raw90
  );
}

/******************************************************************************
  Calibration state
******************************************************************************/

bool calibrationComplete()
{
  return cal.has0 && cal.has45 && cal.has90;
}

void clearSmoothing()
{
  filterInitialized = false;
}

void setCalibrationPoint(float rawAngle, float targetAngle)
{
  if (targetAngle == 0.0)
  {
    cal.raw0 = rawAngle;
    cal.has0 = true;
  }
  else if (targetAngle == 45.0)
  {
    cal.raw45 = rawAngle;
    cal.has45 = true;
  }
  else if (targetAngle == 90.0)
  {
    cal.raw90 = rawAngle;
    cal.has90 = true;
  }
  else
  {
    Serial.println("Unsupported calibration point");
    return;
  }

  saveCalibration();
  clearSmoothing();

  Serial.printf(
    "Saved %.0f degree calibration at raw %.2f\n",
    targetAngle,
    rawAngle
  );
}

void resetCalibration()
{
  cal = CalibrationData();

  saveCalibration();
  clearSmoothing();

  Serial.println("Calibration reset");
}

/******************************************************************************
  Calibration calculations
******************************************************************************/

float mapCalibratedSegment(
  float rawAngle,
  float rawA,
  float rawB,
  float angleA,
  float angleB)
{
  float span = signedDelta(rawA, rawB);

  if (fabs(span) < 1.0)
  {
    return normalizeSigned(rawAngle);
  }

  float position = signedDelta(rawA, rawAngle);

  return angleA +
         position * (angleB - angleA) / span;
}

float applyCalibration(float rawAngle)
{
  /*
    With fewer than two useful points, display the signed raw angle.
  */
  if (!(cal.has0 && cal.has90))
  {
    return normalizeSigned(rawAngle);
  }

  /*
    Two-point calibration using 0 and 90 degrees.
  */
  if (!cal.has45)
  {
    return mapCalibratedSegment(
      rawAngle,
      cal.raw0,
      cal.raw90,
      0.0,
      90.0
    );
  }

  /*
    Three-point calibration.

    The 0-to-45 segment handles values at and below 45 degrees,
    including small negative values below zero.
  */
  float span0to45 = signedDelta(
    cal.raw0,
    cal.raw45
  );

  if (fabs(span0to45) < 1.0)
  {
    return normalizeSigned(rawAngle);
  }

  float positionFromZero = signedDelta(
    cal.raw0,
    rawAngle
  );

  float fraction0to45 =
    positionFromZero / span0to45;

  if (fraction0to45 <= 1.0)
  {
    return mapCalibratedSegment(
      rawAngle,
      cal.raw0,
      cal.raw45,
      0.0,
      45.0
    );
  }

  /*
    Values above 45 degrees use the 45-to-90 segment.
  */
  return mapCalibratedSegment(
    rawAngle,
    cal.raw45,
    cal.raw90,
    45.0,
    90.0
  );
}

/******************************************************************************
  Angle smoothing
******************************************************************************/

float smoothAngle(float newAngle)
{
  if (!filterInitialized)
  {
    smoothedAngle = newAngle;
    filterInitialized = true;

    return smoothedAngle;
  }

  float difference =
    newAngle - smoothedAngle;

  if (difference > 180.0)
  {
    difference -= 360.0;
  }

  if (difference < -180.0)
  {
    difference += 360.0;
  }

  smoothedAngle +=
    SMOOTHING_ALPHA * difference;

  return smoothedAngle;
}

/******************************************************************************
  Serial calibration commands
******************************************************************************/

void handleSerialCommands(float rawAngle)
{
  if (!Serial.available())
  {
    return;
  }

  String command =
    Serial.readStringUntil('\n');

  command.trim();

  if (command == "0")
  {
    setCalibrationPoint(rawAngle, 0.0);
  }
  else if (command == "45")
  {
    setCalibrationPoint(rawAngle, 45.0);
  }
  else if (command == "90")
  {
    setCalibrationPoint(rawAngle, 90.0);
  }
  else if (command == "r" || command == "R")
  {
    resetCalibration();
  }
  else
  {
    Serial.print("Unknown command: ");
    Serial.println(command);
  }
}