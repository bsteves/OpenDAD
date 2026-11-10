OpenDAD 10-point calibration update

Included tabs:
  OpenDAD.ino
  Calibration.ino
  WebServer.ino
  WebPage.ino
  Sensor.ino (unchanged from supplied source)
  Target.ino (unchanged from supplied source)

Install:
1. Back up your current sketch.
2. Replace the matching tabs with the files in this archive.
3. Keep any other project tabs you already use (for example WiFi.ino).
4. Upload the sketch.
5. Reset calibration, then calibrate the mast at 0, 10, 20, ... 90 degrees.
6. Each point is saved independently to ESP32 Preferences as it is set.

The calibration uses all currently saved points, bridges missing points, and
interpolates between adjacent saved raw readings. It extrapolates using the
nearest end segment if fewer than all ten points are set. The UI and serial
monitor both support 10-degree calibration commands.

The WebPage recognizes Safari-downloaded .gcs.xml files as GCS designs.

Checks performed:
- Embedded JavaScript syntax checked with Node.js.
- Static checks for all ten web calibration controls and API routes.
- Arduino compilation and hardware behavior have NOT been tested here.
