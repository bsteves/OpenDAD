# OpenDAD
### Open Digital Angle Display for Gem Faceting Machines

OpenDAD is an open-source Digital Angle Display (DAD) for gem faceting machines.

It combines an ESP32-S3 microcontroller with a TLE5012 magnetic angle sensor to provide a digital angle display accessible from any phone, tablet, or computer using a built-in Wi-Fi web interface.

Unlike commercial DADs costing several hundred dollars, OpenDAD is inexpensive, customizable, and open source.

---

# Features

- Live digital angle display using a TLE5012 magnetic angle sensor
- Seeed Studio XIAO ESP32-S3 controller
- Built-in Wi-Fi access point and browser-based interface
- Responsive interface for phones, tablets, and computers; no app installation required
- Three-point calibration (0°, 45°, 90°), stored in flash memory
- Target angle mode with a prominent signed difference display
- Set a target directly from a selected facet tier in an imported design
- Previous/next facet-tier navigation
- Facet index positions displayed in the tier list and for the selected target
- Import of GemCad ASC and Gem Cut Studio GCS faceting designs
- Three orthographic design previews for GCS files: crown/top, side, and pavilion/bottom
- Selected-tier highlighting in the GCS previews
- Exponential smoothing of angle readings
- Screen Wake Lock request in supported browsers
- REST API for status, calibration, and target control
- Battery operation supported (with suitable battery and wiring)

---

# Hardware

| Component | Notes |
|-----------|------|
| Seeed Studio XIAO ESP32-S3 | Recommended controller |
| TLE5012 sensor | Chinese V1.1 SSC breakout |
| 6 mm diametric magnet | Mounted on the faceting mast |
| 3.7 V LiPo battery (optional) | XIAO ESP32-S3 includes battery charging support; follow Seeed's wiring guidance |
| USB-C cable | Programming and power/charging |

The current design uses the browser interface and does not require an OLED display or Bluetooth module.

---

# Wiring

## TLE5012 sensor

| XIAO ESP32-S3 | TLE5012 |
|---------------|---------|
| D4 | DATA |
| D3 | SCK |
| D2 | CSQ |
| 3.3V | VCC |
| GND | GND |

The current firmware communicates with the TLE5012 using the sensor's SSC-style interface. Use the pin assignments above when wiring the supported breakout.

---

# Web Interface

OpenDAD creates its own Wi-Fi access point. Connect a phone, tablet, or computer to the network:

```text
OpenDAD
```

Then open:

```text
http://192.168.4.1
```

If mDNS is enabled and supported by the client, `http://opendad.local` may also be available.

The main display includes:

- Current calibrated angle and raw sensor angle
- Target angle and signed difference from target
- Difference color feedback
- Manual **Set Target** and **Clear** controls
- Calibration controls
- Sensor/connection status

The page requests the browser Screen Wake Lock where supported. Browser and device power-management settings may still turn off the screen.

## Faceting design import

Use the **Faceting design** file chooser to load a local `.asc` or `.gcs` design file. The file is read and parsed in the browser; it is not uploaded to a remote service or stored permanently by the ESP32.

Supported formats:

- **GemCad ASC (`.asc`)** — tier names, angles, and index positions are parsed from the design instructions. ASC files do not provide the 3D vertex geometry used by the preview, so the geometry views display an explanatory message for ASC designs.
- **Gem Cut Studio GCS (`.gcs`)** — tier names, angles, index positions, instructions, and facet vertex coordinates are read from the XML design. The index gear is read from the file; a default of 96 is used if it is not specified.

The tier list groups entries as Crown, Girdle, or Pavilion based on their tier names. The Table tier (`T`) is not selectable for target setting or adjacent-tier navigation.

### Selecting a facet tier

Select a tier in the list, or use the left/right arrow controls to move to the previous or next selectable tier. Selecting a tier:

1. Highlights that tier in the GCS geometry previews, when geometry is available.
2. Displays its angle as the target angle.
3. Displays the tier's index positions beneath the target angle.
4. Updates the live difference display as the machine approaches the target.

The manual **Set Target** button remains available and sets the target to the current measured angle. **Clear** removes the target.

### Design views

GCS designs with vertex geometry are drawn as three orthographic projections:

- **Face / top (X–Y):** crown facets only; pavilion facets are filtered out.
- **Side (X–Z):** all facet groups are shown to illustrate the stone profile.
- **Bottom / underside:** pavilion facets only; crown facets are filtered out. The Y direction is reversed to represent viewing from below.

The selected tier is highlighted in the views. These are simplified projections of the GCS facet polygons, not a full 3D interactive model. Girdle facets are not included in the top or bottom view filters.

---

# Calibration

OpenDAD uses a three-point calibration:

- Vertical quill → 0°
- Precision angle block → 45°
- Horizontal quill → 90°

This compensates for sensor alignment, magnet placement, mechanical offsets, and small nonlinearities. Calibration is stored in flash memory.

Use the **Set 0°**, **Set 45°**, and **Set 90°** controls when the machine is positioned at the corresponding reference angles. **Reset** clears the calibration points.

---

# Target Mode

A target can be set in either of two ways:

- Press **Set Target** to capture the current measured angle.
- Select a facet tier from an imported design to use that tier's angle as the target.

The display continuously shows the target and the signed difference between the current angle and target. The difference indicator uses these thresholds:

| Absolute difference | Display state |
|---------------------|----------------|
| ≤ 0.05° | Green — at target |
| ≤ 0.20° | Yellow — close |
| ≤ 0.50° | Orange — approaching |
| > 0.50° | Red — farther from target |

The target API accepts angles from 0° through 90°.

---

# REST API

OpenDAD exposes a small HTTP API on the device's web server.

## Read current status

```http
GET /api/status
```

The response includes the calibrated angle, raw sensor reading, calibration state, sensor readiness, connected-device count, and target state. Example:

```json
{
  "angle": 42.16,
  "rawAngle": 218.34,
  "rawWord": 42763,
  "rawHex": "A70B",
  "cal0": true,
  "cal45": true,
  "cal90": true,
  "calibrationComplete": true,
  "sensorReady": true,
  "connectedDevices": 1,
  "targetSet": true,
  "target": 43.00,
  "difference": -0.84
}
```

Values above are illustrative.

## Calibration

```http
POST /api/calibrate/0
POST /api/calibrate/45
POST /api/calibrate/90
POST /api/reset
```

## Target

Set the target to the current measured angle:

```http
POST /api/target
```

Set a target to a specified facet angle (0–90 degrees):

```http
POST /api/target?angle=43.25
```

Clear the target:

```http
POST /api/target/clear
```

Invalid, non-numeric, or out-of-range target angles are rejected with an HTTP 400 response.

---

# Smoothing

OpenDAD uses an exponential moving average to smooth sensor readings:

```cpp
filtered = filtered + α * (new - filtered);
```

The smoothing factor is configurable in the firmware.

---

# Software Structure

The Arduino sketch is organized into tabs/modules:

```text
OpenDAD.ino       Main setup and loop
Sensor.ino        TLE5012 communication and angle reading
Calibration.ino   Three-point calibration and persistence
Target.ino        Target-angle state and difference calculation
WiFi.ino          Wi-Fi access-point setup
WebServer.ino     HTTP routes and REST API
WebPage.ino       Embedded HTML, CSS, and JavaScript interface
```

`WebPage.ino` contains the browser interface as a `PROGMEM` string. `WebServer.ino` serves that page and handles the API routes. The ASC/GCS parsing and design visualization run in the browser-side JavaScript embedded in the page.

---

# Required Arduino Libraries

- ESP32 Arduino Core (including the XIAO ESP32-S3 board support)
- `Preferences` (included with the ESP32 Arduino core)
- `WebServer` (included with the ESP32 Arduino core)

No external web-server or JavaScript libraries are required by the current interface. The browser uses built-in APIs, including `DOMParser` for GCS XML parsing and SVG for the design previews.

---

# Building and Uploading

1. Install the ESP32 Arduino core and Seeed XIAO ESP32-S3 board support.
2. Open the OpenDAD Arduino sketch with all project `.ino` tabs in the same sketch folder.
3. Select **Seeed Studio XIAO ESP32-S3** as the board.
4. Install or enable the required ESP32 core libraries.
5. Wire the TLE5012 sensor using the pin table above.
6. Compile and upload the sketch.
7. Connect a phone, tablet, or computer to the **OpenDAD** Wi-Fi network.
8. Open `http://192.168.4.1`.
9. Perform the 0° / 45° / 90° calibration.
10. Optionally load a `.asc` or `.gcs` design file and select a facet tier to set its target angle.

The current browser UI and API files are evolving. Confirm the firmware compiles with the installed ESP32 board package before using it on the machine; this README does not imply a hardware validation of every feature or board revision.

---

# Roadmap

Potential future improvements include:

- 3D-printable mounting hardware and enclosure refinements
- Audible target indicator
- Guided calibration wizard
- Web-based settings page
- Battery-level display and power-management refinements
- OTA firmware updates and an automatic update checker
- Data logging
- Configurable display themes
- More complete design-format support and richer design visualization

---

# License

MIT License

---

# Acknowledgements

Special thanks to:

- Infineon Technologies
- The gem faceting community
- Everyone contributing ideas and testing OpenDAD

---

# Project Status

**Current Version:** 2.0 (Development)

OpenDAD has evolved from an earlier Bluetooth/OLED prototype into a Wi-Fi-connected digital instrument with a responsive browser interface. The current development includes local GemCad ASC and Gem Cut Studio GCS design import, facet-tier target selection, index-position display, and three-view GCS geometry previews. Hardware validation, battery/enclosure refinements, and continued interface testing remain ongoing.

Contributions, testing, feature requests, and bug reports are welcome.
