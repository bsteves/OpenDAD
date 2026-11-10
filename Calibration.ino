/******************************************************************************
  OpenDAD 10-point calibration, smoothing, storage, and serial commands
******************************************************************************/

#include <math.h>

/******************************************************************************
  Global calibration and Preferences objects
******************************************************************************/

Preferences prefs;
CalibrationData cal;

float smoothedAngle = 0.0;
bool filterInitialized = false;

const float SMOOTHING_ALPHA = 0.3f;
const int CAL_POINT_COUNT = 10;
const float CAL_TARGET_ANGLES[CAL_POINT_COUNT] = {
  0.0f, 10.0f, 20.0f, 30.0f, 40.0f,
  50.0f, 60.0f, 70.0f, 80.0f, 90.0f
};

/******************************************************************************
  Angle utility functions
******************************************************************************/

float normalizeSigned(float angle)
{
  while (angle > 180.0f) angle -= 360.0f;
  while (angle <= -180.0f) angle += 360.0f;
  return angle;
}

float signedDelta(float fromAngle, float toAngle)
{
  return normalizeSigned(toAngle - fromAngle);
}

/******************************************************************************
  Calibration point lookup and storage
******************************************************************************/

int calibrationIndex(float targetAngle)
{
  int index = (int)lroundf(targetAngle / 10.0f);

  if (index < 0 || index >= CAL_POINT_COUNT) return -1;
  if (fabsf(targetAngle - CAL_TARGET_ANGLES[index]) > 0.01f) return -1;

  return index;
}

bool calibrationPointSet(float targetAngle)
{
  int index = calibrationIndex(targetAngle);
  return index >= 0 && cal.has[index];
}

void saveCalibration()
{
  for (int i = 0; i < CAL_POINT_COUNT; i++)
  {
    char rawKey[12];
    char setKey[12];

    // Use new key names so legacy cal0/cal45/cal90 values cannot be
    // mistaken for a completed ten-point calibration.
    snprintf(rawKey, sizeof(rawKey), "praw%d", i * 10);
    snprintf(setKey, sizeof(setKey), "pset%d", i * 10);

    prefs.putFloat(rawKey, cal.raw[i]);
    prefs.putBool(setKey, cal.has[i]);
  }
}

void loadCalibration()
{
  for (int i = 0; i < CAL_POINT_COUNT; i++)
  {
    char rawKey[12];
    char setKey[12];

    snprintf(rawKey, sizeof(rawKey), "praw%d", i * 10);
    snprintf(setKey, sizeof(setKey), "pset%d", i * 10);

    cal.raw[i] = prefs.getFloat(rawKey, CAL_TARGET_ANGLES[i]);
    cal.has[i] = prefs.getBool(setKey, false);
  }

  Serial.println("10-point calibration loaded:");

  for (int i = 0; i < CAL_POINT_COUNT; i++)
  {
    Serial.printf(
      "  %2d degrees: %-7s raw %.3f\n",
      i * 10,
      cal.has[i] ? "set" : "not set",
      cal.raw[i]
    );
  }

  Serial.printf(
    "Calibration complete: %s\n",
    calibrationComplete() ? "YES" : "NO"
  );
}

/******************************************************************************
  Calibration state
******************************************************************************/

bool calibrationComplete()
{
  for (int i = 0; i < CAL_POINT_COUNT; i++)
  {
    if (!cal.has[i]) return false;
  }

  return true;
}

void clearSmoothing()
{
  filterInitialized = false;
}

void setCalibrationPoint(float rawAngle, float targetAngle)
{
  int index = calibrationIndex(targetAngle);

  if (index < 0)
  {
    Serial.println("Unsupported calibration point. Use 0, 10, 20 ... 90.");
    return;
  }

  // Capture the raw sensor reading, never the calibrated or smoothed angle.
  cal.raw[index] = rawAngle;
  cal.has[index] = true;

  saveCalibration();
  clearSmoothing();

  Serial.printf(
    "Saved %d degree calibration at raw %.3f\n",
    index * 10,
    rawAngle
  );

  int savedCount = 0;
  for (int i = 0; i < CAL_POINT_COUNT; i++)
  {
    if (cal.has[i]) savedCount++;
  }

  Serial.printf(
    "Calibration points saved: %d/%d\n",
    savedCount,
    CAL_POINT_COUNT
  );
}

void resetCalibration()
{
  cal = CalibrationData();

  saveCalibration();
  clearSmoothing();

  Serial.println("10-point calibration reset");
}

/******************************************************************************
  Calibration calculation

  Uses all saved points in target-angle order. Raw readings are unwrapped
  relative to the first saved point, allowing the sensor's 0/360 transition.
  Missing calibration points are bridged by the neighboring saved points.
  Values outside the saved range use the nearest end segment for extrapolation.
******************************************************************************/

float applyCalibration(float rawAngle)
{
  int validIndices[CAL_POINT_COUNT];
  int validCount = 0;

  for (int i = 0; i < CAL_POINT_COUNT; i++)
  {
    if (cal.has[i]) validIndices[validCount++] = i;
  }

  // At least two independent reference points are required.
  if (validCount < 2)
  {
    return normalizeSigned(rawAngle);
  }

  float rawOffsets[CAL_POINT_COUNT];
  rawOffsets[0] = 0.0f;

  float firstSpan = signedDelta(
    cal.raw[validIndices[0]],
    cal.raw[validIndices[1]]
  );

  if (fabsf(firstSpan) < 0.01f)
  {
    return normalizeSigned(rawAngle);
  }

  // Determine whether raw sensor angle increases or decreases with mast angle.
  const float direction = (firstSpan >= 0.0f) ? 1.0f : -1.0f;

  for (int i = 1; i < validCount; i++)
  {
    float span = signedDelta(
      cal.raw[validIndices[i - 1]],
      cal.raw[validIndices[i]]
    ) * direction;

    // A non-monotonic or duplicated calibration reading is not safe to map.
    if (span <= 0.01f)
    {
      return normalizeSigned(rawAngle);
    }

    rawOffsets[i] = rawOffsets[i - 1] + span;
  }

  float rawPosition = signedDelta(
    cal.raw[validIndices[0]],
    rawAngle
  ) * direction;

  // Find the segment containing this raw position. Extrapolate with the
  // first or last segment when the angle is outside the calibrated range.
  int segment = 0;

  if (rawPosition >= rawOffsets[validCount - 1])
  {
    segment = validCount - 2;
  }
  else if (rawPosition > rawOffsets[0])
  {
    for (int i = 0; i < validCount - 1; i++)
    {
      if (rawPosition <= rawOffsets[i + 1])
      {
        segment = i;
        break;
      }
    }
  }

  int indexA = validIndices[segment];
  int indexB = validIndices[segment + 1];

  float rawSpan = rawOffsets[segment + 1] - rawOffsets[segment];
  if (rawSpan <= 0.01f)
  {
    return normalizeSigned(rawAngle);
  }

  float fraction =
    (rawPosition - rawOffsets[segment]) / rawSpan;

  return CAL_TARGET_ANGLES[indexA] +
         fraction * (CAL_TARGET_ANGLES[indexB] - CAL_TARGET_ANGLES[indexA]);
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

  float difference = newAngle - smoothedAngle;

  if (difference > 180.0f) difference -= 360.0f;
  if (difference < -180.0f) difference += 360.0f;

  smoothedAngle += SMOOTHING_ALPHA * difference;
  return smoothedAngle;
}

/******************************************************************************
  Serial calibration commands
******************************************************************************/

void handleSerialCommands(float rawAngle)
{
  if (!Serial.available()) return;

  String command = Serial.readStringUntil('\n');
  command.trim();

  if (command == "r" || command == "R")
  {
    resetCalibration();
    return;
  }

  for (int i = 0; i < CAL_POINT_COUNT; i++)
  {
    String expected = String(i * 10);

    if (command == expected)
    {
      setCalibrationPoint(rawAngle, CAL_TARGET_ANGLES[i]);
      return;
    }
  }

  Serial.print("Unknown command: ");
  Serial.println(command);
}
