/******************************************************************************
  OpenDAD web server

  Provides:
    GET  /                  OpenDAD webpage
    GET  /api/status        Current angle and ten-point calibration status
    POST /api/calibrate/0..90  Save a calibration point in 10-degree steps
    POST /api/reset         Clear calibration

  The ESP32 built-in WebServer library handles one HTTP request at a time,
  which is sufficient for the OpenDAD interface.
******************************************************************************/

#include <WebServer.h>
#include <stdlib.h>
#include <math.h>

/******************************************************************************
  Web-server object
******************************************************************************/

WebServer server(80);

/******************************************************************************
  Current instrument state

  loop() updates these values using updateWebState(). Web requests read them.
******************************************************************************/

struct WebInstrumentState
{
  uint16_t rawWord = 0;
  float rawAngle = 0.0;
  float displayAngle = 0.0;
  bool sensorReady = false;
};

WebInstrumentState webState;

/******************************************************************************
  Web page

  WEB_PAGE is defined in WebPage.ino. Arduino compiles all .ino tabs in the
  sketch together.
******************************************************************************/

extern const char WEB_PAGE[] PROGMEM;


/******************************************************************************
  Instrument-state update

  Call this from loop() after reading and processing the sensor.
******************************************************************************/

void updateWebState(
  uint16_t rawWord,
  float rawAngle,
  float displayAngle)
{
  webState.rawWord = rawWord;
  webState.rawAngle = rawAngle;
  webState.displayAngle = displayAngle;
  webState.sensorReady = true;
}

/******************************************************************************
  HTTP response helpers
******************************************************************************/

void sendJsonResponse(const String& json, int statusCode = 200)
{
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Pragma", "no-cache");
  server.send(statusCode, "application/json", json);
}

void sendCommandResult(
  bool success,
  const char* message,
  int statusCode = 200)
{
  String json;
  json.reserve(100);

  json += "{\"success\":";
  json += success ? "true" : "false";
  json += ",\"message\":\"";
  json += message;
  json += "\"}";

  sendJsonResponse(json, statusCode);
}

/******************************************************************************
  Web-page handler
******************************************************************************/

void handleRoot()
{
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html", WEB_PAGE);
}

/******************************************************************************
  Status API

  Example response:

  {
    "angle": 42.18,
    "rawAngle": 218.34,
    "rawWord": 42763,
    "rawHex": "A70B",
    " cal0": true,
    "cal45": true,
    "cal90": false,
    "calibrationComplete": false,
    "sensorReady": true
  }
******************************************************************************/

void handleApiStatus()
{
  String json;
  json.reserve(520);

  json += "{";
  json += "\"angle\":" + String(webState.displayAngle, 2) + ",";
  json += "\"rawAngle\":" + String(webState.rawAngle, 2) + ",";
  json += "\"rawWord\":" + String(webState.rawWord) + ",";
  json += "\"rawHex\":\"";
  char rawHex[5];
  snprintf(rawHex, sizeof(rawHex), "%04X", webState.rawWord);
  json += rawHex;
  json += "\",";
  json += "\"calibrationPoints\":[";

  for (int i = 0; i <= 90; i += 10)
  {
    if (i > 0) json += ",";
    json += calibrationPointSet((float)i) ? "true" : "false";
  }

  json += "],";
  json += "\"calibrationComplete\":";
  json += calibrationComplete() ? "true" : "false";
  json += ",";
  json += "\"sensorReady\":";
  json += webState.sensorReady ? "true" : "false";
  json += ",";
  json += "\"connectedDevices\":" + String(getConnectedDeviceCount()) + ",";
  json += "\"targetSet\":";
  json += targetAngleSet ? "true" : "false";
  json += ",";
  json += "\"target\":" + String(targetAngle, 2) + ",";
  json += "\"difference\":" + String(getTargetDifference(), 2);
  json += "}";

  sendJsonResponse(json);
}

/******************************************************************************
  Calibration API handler

  Route suffix must be one of 0, 10, 20 ... 90.
******************************************************************************/

void handleCalibratePoint()
{
  if (!webState.sensorReady)
  {
    sendCommandResult(false, "Sensor data is not available", 503);
    return;
  }

  const String prefix = "/api/calibrate/";
  String uri = server.uri();

  if (!uri.startsWith(prefix))
  {
    sendCommandResult(false, "Invalid calibration route", 400);
    return;
  }

  String angleText = uri.substring(prefix.length());
  angleText.trim();

  if (angleText.length() == 0)
  {
    sendCommandResult(false, "Missing calibration angle", 400);
    return;
  }

  for (unsigned int i = 0; i < angleText.length(); i++)
  {
    if (!isDigit(angleText[i]))
    {
      sendCommandResult(false, "Invalid calibration angle", 400);
      return;
    }
  }

  int target = angleText.toInt();

  if (target < 0 || target > 90 || (target % 10) != 0)
  {
    sendCommandResult(
      false,
      "Calibration angle must be 0 through 90 in 10-degree steps",
      400
    );
    return;
  }

  setCalibrationPoint(webState.rawAngle, (float)target);

  String message = "Calibration saved at ";
  message += String(target);
  message += " degrees";
  sendCommandResult(true, message.c_str());
}

void handleResetCalibration()
{
  resetCalibration();
  sendCommandResult(true, "Calibration cleared");
}

/******************************************************************************
  Target setting
******************************************************************************/

void handleSetTarget()
{
  // A supplied angle is used when the user selects a facet tier from
  // an imported ASC/GCS design. Without an angle, retain the original
  // behavior of setting the target to the current instrument angle.
  if (server.hasArg("angle"))
  {
    String angleText = server.arg("angle");
    angleText.trim();

    if (angleText.length() == 0)
    {
      sendCommandResult(false, "Missing target angle", 400);
      return;
    }

    // Reject malformed values instead of allowing String::toFloat()
    // to silently turn them into 0 degrees.
    char *endPointer = nullptr;
    float requestedAngle = strtof(angleText.c_str(), &endPointer);
    if (endPointer == angleText.c_str() || *endPointer != '\0' ||
        !isfinite(requestedAngle))
    {
      sendCommandResult(false, "Invalid target angle", 400);
      return;
    }

    // Faceting target angles are expressed from 0 through 90 degrees.
    if (requestedAngle < 0.0f || requestedAngle > 90.0f)
    {
      sendCommandResult(
        false,
        "Target angle must be between 0 and 90 degrees",
        400
      );
      return;
    }

    targetAngle = requestedAngle;
    targetAngleSet = true;
  }
  else
  {
    // Preserve the manual Set Target button behavior.
    setTargetToCurrentAngle();
  }

  String json = "{";
  json += "\"success\":true,";
  json += "\"target\":" + String(targetAngle, 2);
  json += "}";

  sendJsonResponse(json);
}


void handleClearTarget()
{
  clearTargetAngle();

  server.send(
    200,
    "application/json",
    "{\"success\":true}"
  );
}





/******************************************************************************
  Not-found handler
******************************************************************************/

void handleNotFound()
{
  String json;
  json.reserve(120);

  json += "{\"success\":false,\"message\":\"Not found\",";
  json += "\"path\":\"";
  json += server.uri();
  json += "\"}";

  sendJsonResponse(json, 404);
}

/******************************************************************************
  Start the HTTP server
******************************************************************************/

void setupWebServer()
{
  /*
    GET routes
  */
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleApiStatus);

  /*
    POST routes
  */
  for (int angle = 0; angle <= 90; angle += 10)
  {
    String route = "/api/calibrate/" + String(angle);
    server.on(route, HTTP_POST, handleCalibratePoint);
  }

  server.on(
    "/api/reset",
    HTTP_POST,
    handleResetCalibration
  );

  server.on(
    "/api/target",
    HTTP_POST,
    handleSetTarget
  );
  
  server.on(
    "/api/target/clear",
    HTTP_POST,
    handleClearTarget
  );


  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println();
  Serial.println("OpenDAD web server started");
  Serial.print("Open http://");
  Serial.println(WiFi.softAPIP());
}

/******************************************************************************
  Process incoming web requests

  Call frequently from loop().
******************************************************************************/

void handleWebServer()
{
  server.handleClient();
}