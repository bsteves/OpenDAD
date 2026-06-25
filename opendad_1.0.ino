/******************************************************************************
  Open Digital Angle Display (DAD) for Faceting Machines

  Hardware:
    - ESP32
    - TLE5012 magnetic angle sensor breakout
    - SSD1306 128x64 I2C OLED
    - RemoteXY BLE phone interface

  Calibration:
    - 0 button  = current quill position is 0 degrees
    - 45 button = current quill position is 45 degrees
    - 90 button = current quill position is 90 degrees
    - Reset     = clear calibration

  Serial commands:
    0, 45, 90, r

******************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/******************************************************************************
  RemoteXY configuration
******************************************************************************/

#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#define REMOTEXY_BLUETOOTH_NAME "RemoteXY"
#include <RemoteXY.h>

#pragma pack(push, 1)
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =
  { 255,4,0,4,0,73,0,19,0,0,0,69,83,80,51,50,0,31,1,106,
  200,1,1,5,0,1,44,62,15,15,0,120,31,52,53,0,1,69,62,15,
  15,0,6,31,57,48,0,1,17,63,15,15,0,36,31,48,0,67,17,100,
  78,28,78,2,26,3,1,39,25,24,24,1,60,31,82,101,115,101,116,0 };

struct {
  uint8_t fortyfive_btn;
  uint8_t ninety_btn;
  uint8_t zero_btn;
  uint8_t reset_btn;
  float angle_text;
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

/******************************************************************************
  User-adjustable settings
******************************************************************************/

const float SMOOTHING_ALPHA = 0.25;  // higher = faster, lower = smoother

/******************************************************************************
  Hardware pins
******************************************************************************/

const int DATA_PIN = 23;
const int SCK_PIN  = 18;
const int CS_PIN   = 5;

const int OLED_SDA = 21;
const int OLED_SCL = 22;

const int SCREEN_WIDTH  = 128;
const int SCREEN_HEIGHT = 64;
const int OLED_ADDR     = 0x3C;

const uint16_t CMD_READ_ANGLE = 0x8021;

/******************************************************************************
  Global objects and state
******************************************************************************/

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Preferences prefs;

struct CalibrationData {
  float raw0 = 0.0;
  float raw45 = 45.0;
  float raw90 = 90.0;
  bool has0 = false;
  bool has45 = false;
  bool has90 = false;
};

struct ButtonState {
  bool lastZero = false;
  bool lastFortyFive = false;
  bool lastNinety = false;
  bool lastReset = false;
};

CalibrationData cal;
ButtonState buttons;

float smoothedAngle = 0.0;
bool filterInitialized = false;

/******************************************************************************
  Utility functions
******************************************************************************/

float normalizeSigned(float angle)
{
  while (angle > 180.0) angle -= 360.0;
  while (angle <= -180.0) angle += 360.0;
  return angle;
}

float signedDelta(float fromAngle, float toAngle)
{
  return normalizeSigned(toAngle - fromAngle);
}

/******************************************************************************
  TLE5012 bit-banged SSC communication
******************************************************************************/

uint16_t sscTransfer16(uint16_t tx)
{
  uint16_t rx = 0;

  pinMode(DATA_PIN, OUTPUT);

  for (int bit = 15; bit >= 0; bit--)
  {
    digitalWrite(DATA_PIN, (tx >> bit) & 1);
    delayMicroseconds(2);

    digitalWrite(SCK_PIN, HIGH);
    delayMicroseconds(5);

    digitalWrite(SCK_PIN, LOW);
    delayMicroseconds(5);
  }

  pinMode(DATA_PIN, INPUT_PULLUP);
  delayMicroseconds(5);

  for (int bit = 15; bit >= 0; bit--)
  {
    digitalWrite(SCK_PIN, HIGH);
    delayMicroseconds(5);

    if (digitalRead(DATA_PIN))
      rx |= (1 << bit);

    digitalWrite(SCK_PIN, LOW);
    delayMicroseconds(5);
  }

  return rx;
}

uint16_t readSensorRawWord()
{
  digitalWrite(CS_PIN, LOW);
  delayMicroseconds(5);

  uint16_t raw = sscTransfer16(CMD_READ_ANGLE);

  digitalWrite(CS_PIN, HIGH);
  delayMicroseconds(5);

  return raw;
}

float rawWordToDegrees(uint16_t raw)
{
  int16_t signedRaw = (int16_t)raw;
  float degrees = signedRaw * 360.0 / 32768.0;

  if (degrees < 0.0)
    degrees += 360.0;

  return degrees;
}

/******************************************************************************
  Calibration
******************************************************************************/

bool calibrationComplete()
{
  return cal.has0 && cal.has45 && cal.has90;
}

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

  saveCalibration();
  clearSmoothing();

  Serial.printf("Saved %.0f deg calibration at raw %.2f\n",
                targetAngle, rawAngle);
}

void resetCalibration()
{
  cal = CalibrationData();
  saveCalibration();
  clearSmoothing();
  Serial.println("Calibration reset");
}

float applyCalibration(float rawAngle)
{
  if (!calibrationComplete())
    return normalizeSigned(rawAngle);

  float span0to45 = signedDelta(cal.raw0, cal.raw45);
  float span45to90 = signedDelta(cal.raw45, cal.raw90);

  if (fabs(span0to45) < 1.0 || fabs(span45to90) < 1.0)
    return normalizeSigned(rawAngle);

  float pos0 = signedDelta(cal.raw0, rawAngle);

  if (pos0 <= span0to45)
    return pos0 * 45.0 / span0to45;

  float pos45 = signedDelta(cal.raw45, rawAngle);
  return 45.0 + pos45 * 45.0 / span45to90;
}

/******************************************************************************
  Smoothing
******************************************************************************/

float smoothAngle(float newAngle)
{
  if (!filterInitialized)
  {
    smoothedAngle = newAngle;
    filterInitialized = true;
    return smoothedAngle;
  }

  float diff = newAngle - smoothedAngle;

  if (diff > 180.0) diff -= 360.0;
  if (diff < -180.0) diff += 360.0;

  smoothedAngle += SMOOTHING_ALPHA * diff;
  return smoothedAngle;
}

/******************************************************************************
  Input handling
******************************************************************************/

void handleSerialCommands(float rawAngle)
{
  if (!Serial.available())
    return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd == "0")
    setCalibrationPoint(rawAngle, 0.0);
  else if (cmd == "45")
    setCalibrationPoint(rawAngle, 45.0);
  else if (cmd == "90")
    setCalibrationPoint(rawAngle, 90.0);
  else if (cmd == "r")
    resetCalibration();
}

void handleRemoteXYButtons(float rawAngle)
{
  bool zeroNow = RemoteXY.zero_btn;
  bool fortyFiveNow = RemoteXY.fortyfive_btn;
  bool ninetyNow = RemoteXY.ninety_btn;
  bool resetNow = RemoteXY.reset_btn;

  if (zeroNow && !buttons.lastZero)
    setCalibrationPoint(rawAngle, 0.0);

  if (fortyFiveNow && !buttons.lastFortyFive)
    setCalibrationPoint(rawAngle, 45.0);

  if (ninetyNow && !buttons.lastNinety)
    setCalibrationPoint(rawAngle, 90.0);

  if (resetNow && !buttons.lastReset)
    resetCalibration();

  buttons.lastZero = zeroNow;
  buttons.lastFortyFive = fortyFiveNow;
  buttons.lastNinety = ninetyNow;
  buttons.lastReset = resetNow;
}

/******************************************************************************
  Display
******************************************************************************/

void updateOLED(float angle, uint16_t rawWord)
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);

  if (calibrationComplete())
    display.println("DAD CAL:OK");
  else
    display.println("DAD CAL:SET 0/45/90");

  display.setTextSize(3);
  display.setCursor(0, 20);
  display.print(angle, 2);
  display.print((char)247);

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("RAW ");
  display.print(rawWord, HEX);

  display.display();
}

void updateRemoteXY(float angle)
{
  RemoteXY.angle_text = angle;
}

/******************************************************************************
  System setup
******************************************************************************/

void setupPins()
{
  pinMode(CS_PIN, OUTPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT_PULLUP);

  digitalWrite(CS_PIN, HIGH);
  digitalWrite(SCK_PIN, LOW);
}

void setupOLED()
{
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("OLED not found");

    while (true)
      RemoteXYEngine.delay(100);
  }

  display.clearDisplay();
  display.display();
}

void printStartupInfo()
{
  Serial.println();
  Serial.println("Open DAD for Faceting Machines");
  Serial.println("TLE5012 + OLED + RemoteXY");
  Serial.println();
  Serial.println("Calibration:");
  Serial.println("  0  = save current position as 0 degrees");
  Serial.println("  45 = save current position as 45 degrees");
  Serial.println("  90 = save current position as 90 degrees");
  Serial.println("  r  = reset calibration");
  Serial.println();
}

/******************************************************************************
  Arduino setup and loop
******************************************************************************/

void setup()
{
  Serial.begin(115200);
  delay(1000);

  RemoteXY_Init();

  prefs.begin("dad", false);
  loadCalibration();

  setupPins();
  setupOLED();
  printStartupInfo();
}

void loop()
{
  RemoteXYEngine.handler();

  uint16_t rawWord = readSensorRawWord();
  float rawAngle = rawWordToDegrees(rawWord);

  handleSerialCommands(rawAngle);
  handleRemoteXYButtons(rawAngle);

  float calibrated = applyCalibration(rawAngle);
  float filtered = smoothAngle(calibrated);

  updateRemoteXY(filtered);
  updateOLED(filtered, rawWord);

  Serial.printf("RAW: 0x%04X  RAW_ANGLE: %.2f  DISPLAY: %.2f\n",
                rawWord, rawAngle, filtered);

  RemoteXYEngine.delay(50);
}
