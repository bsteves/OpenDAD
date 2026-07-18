# OpenDAD
### Open Digital Angle Display for Gem Faceting Machines

OpenDAD is an open-source Digital Angle Display (DAD) for gem faceting machines.

It combines an ESP32-S3 microcontroller with a TLE5012 magnetic angle sensor to provide a highly accurate digital angle display accessible from any phone, tablet, or computer using a built-in Wi-Fi web interface.

Unlike commercial DADs costing several hundred dollars, OpenDAD is inexpensive, fully customizable, and completely open source.

---

# Features

- High-resolution digital angle display
- TLE5012 magnetic angle sensor
- Seeed Studio XIAO ESP32-S3 controller
- Built-in Wi-Fi web interface
- Works from any phone, tablet, or browser
- Three-point calibration (0°, 45°, 90°)
- Automatic calibration storage in flash memory
- Target angle mode
- Large live angle difference display
- Exponential smoothing
- REST API for future integrations
- Battery operation supported
- Open-source hardware and software

---

# Hardware

| Component | Notes |
|-----------|------|
| Seeed Studio XIAO ESP32-S3 | Recommended controller |
| TLE5012 Sensor | Chinese V1.1 SSC breakout |
| 6 mm diametric magnet | Mounted on faceting mast |
| 3.7V LiPo battery (optional) | XIAO includes charger |
| USB-C | Programming and charging |

The project no longer requires an OLED display or Bluetooth module.

---

# Wiring

## TLE5012

| XIAO ESP32-S3 | TLE5012 |
|---------------|----------|
| D4 | DATA |
| D3 | SCK |
| D2 | CSQ |
| 3.3V | VCC |
| GND | GND |

---

# Web Interface

OpenDAD creates its own Wi-Fi Access Point.

Simply connect your phone to:

```
OpenDAD
```

and browse to

```
http://192.168.4.1
```

(or `http://opendad.local` if mDNS is enabled).

The browser interface displays

- Current angle
- Raw sensor angle
- Target angle
- Target difference
- Calibration controls
- Connection status

No app installation is required.

---

# Calibration

OpenDAD uses a three-point calibration:

- Vertical quill → 0°
- Precision angle block → 45°
- Horizontal quill → 90°

This compensates for

- Sensor alignment
- Magnet placement
- Mechanical offsets
- Small nonlinearities

Calibration is automatically stored in flash memory.

---

# Target Mode

A target angle can be stored at any time.

The display continuously shows:

- Target angle
- Signed difference from target

Difference colors indicate proximity:

| Difference | Color |
|------------|------|
| ≤0.05° | Green |
| ≤0.20° | Yellow |
| ≤0.50° | Orange |
| >0.50° | Red |

This makes approaching an angle much easier while cutting.

---

# REST API

OpenDAD exposes a simple REST interface.

## Read current status

```
GET /api/status
```

Example

```json
{
  "angle": 42.16,
  "rawAngle": 218.34,
  "target": 43.00,
  "difference": -0.84,
  "cal0": true,
  "cal45": true,
  "cal90": true,
  "sensorReady": true
}
```

## Calibration

```
POST /api/calibrate/0
POST /api/calibrate/45
POST /api/calibrate/90
POST /api/reset
```

## Target

```
POST /api/target
POST /api/target/clear
```

---

# Smoothing

OpenDAD uses an exponential moving average

```cpp
filtered = filtered + α(new - filtered)
```

The smoothing factor is configurable in software.

---

# Software Structure

```
OpenDAD.ino
Sensor.ino
Calibration.ino
Target.ino
WiFi.ino
WebServer.ino
WebPage.ino
```

Each module has a single responsibility, making the project easier to maintain and extend.

---

# Required Arduino Libraries

- ESP32 Arduino Core
- Preferences

No external web libraries are required.

---

# Building

1. Install the ESP32 Arduino core.
2. Select **Seeed Studio XIAO ESP32-S3**.
3. Install the required libraries.
4. Wire the TLE5012.
5. Compile and upload.
6. Connect to the OpenDAD Wi-Fi network.
7. Open the web interface.
8. Perform the 0° / 45° / 90° calibration.

---

# Roadmap

Future ideas include:

- 3D printable mounting hardware
- Audible target indicator
- Calibration wizard
- Web-based settings page
- Battery level display
- Wake Lock support for mobile browsers
- OTA firmware updates
- Automatic firmware update checker
- Data logging
- Configurable display themes

---

# License

MIT License

---

# Acknowledgements

Special thanks to

- Infineon Technologies
- The gem faceting community
- Everyone contributing ideas and testing OpenDAD

---

# Project Status

**Current Version:** 2.0 (Development)

OpenDAD has evolved from a Bluetooth/OLED prototype into a Wi-Fi connected digital instrument with a responsive web interface. The core angle measurement, calibration, and target functions are complete, and the project is now focused on polishing the user interface, battery operation, and enclosure design.

Contributions, testing, feature requests, and bug reports are welcome.