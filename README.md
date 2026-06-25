# Open Digital Angle Display (DAD) for Faceting Machines

An open-source Bluetooth-enabled Digital Angle Display (DAD) for gem faceting machines using an ESP32, a TLE5012 magnetic angle sensor, and a small OLED display.

The goal of this project is to provide an inexpensive, accurate, and customizable alternative to commercial digital angle displays. The entire system can be built for well under $50 using readily available components.

---

## Features

* ±0.01° display resolution
* TLE5012 magnetic angle sensor
* ESP32 microcontroller
* SSD1306 128×64 OLED display
* Bluetooth connectivity using RemoteXY
* Three-point calibration (0°, 45°, and 90°)
* Calibration automatically saved in ESP32 flash memory
* Exponential angle smoothing
* Signed angle display (-20° to approximately 100°)
* Open-source Arduino code
* No custom PCB required

---

## Hardware

| Component       | Notes                                                |
| --------------- | ---------------------------------------------------- |
| ESP32 Dev Board | Any ESP32-WROOM development board                    |
| TLE5012 Sensor  | Chinese V1.1 breakout board using SSC interface      |
| OLED Display    | SSD1306 128×64 I²C (0.96")                           |
| Magnet          | 6 mm diametric magnet (mounted on the faceting mast) |
| Power           | USB                                                  |

---

## Wiring

### TLE5012

| ESP32  | TLE5012 |
| ------ | ------- |
| GPIO23 | DATA    |
| GPIO18 | SCK     |
| GPIO5  | CS      |
| 3.3V   | VCC     |
| GND    | GND     |

### OLED

| ESP32  | OLED |
| ------ | ---- |
| GPIO21 | SDA  |
| GPIO22 | SCL  |
| 3.3V   | VCC  |
| GND    | GND  |

---

## Why Bit-Banged SPI?

Although the TLE5012 uses an SPI-like SSC (Short Serial Communication) interface, the inexpensive Chinese V1.1 breakout boards have proven unreliable using the ESP32 hardware SPI peripheral.

Instead, this project implements the SSC protocol entirely in software ("bit banging"), which has proven to be extremely reliable.

---

## Bluetooth Interface

The project uses the excellent **RemoteXY** library to communicate with an iPhone or Android device over Bluetooth Low Energy (BLE).

Current controls include:

* **0°** — Store the current quill position as vertical (0°)
* **45°** — Store the current quill position as 45°
* **90°** — Store the current quill position as horizontal (90°)
* **Reset** — Clear all calibration data

The current angle is displayed on both the OLED display and the RemoteXY application.

---

## Calibration

Unlike many digital angle displays that only provide a single zero offset, this project performs a three-point calibration.

Calibration points:

* Vertical quill → 0°
* 45° test block → 45°
* Horizontal quill → 90°

These three reference points compensate for:

* Sensor mounting angle
* Magnet alignment
* Mechanical tolerances
* Slight nonlinearities

Calibration data is stored in the ESP32 flash memory and automatically restored after power cycling.

---

## Display

The OLED displays:

* Current angle (two decimal places)
* Calibration status
* Raw sensor value (for debugging)

The displayed angle is normalized so that values near 360° are shown as small negative angles.

For example:

| Raw Sensor | Display |
| ---------- | ------- |
| 359.82°    | -0.18°  |
| 359.05°    | -0.95°  |
| 0.15°      | 0.15°   |

This makes it much easier to work around the common 0° cutting position.

---

## Smoothing

An exponential moving average (EMA) is used to reduce display jitter.

```cpp
filtered = filtered + α(new - filtered)
```

The smoothing factor is controlled by:

```cpp
const float SMOOTHING_ALPHA = 0.25;
```

Suggested values:

| Alpha | Behavior          |
| ----- | ----------------- |
| 0.10  | Very smooth       |
| 0.20  | Smooth            |
| 0.25  | Good compromise   |
| 0.30  | Fast response     |
| 0.40  | Minimal filtering |

---

## Arduino Libraries

Install the following libraries through the Arduino Library Manager:

* Adafruit GFX
* Adafruit SSD1306
* Preferences (ESP32)
* RemoteXY
* BLE (included with ESP32 core)

---

## Building

1. Install the ESP32 Arduino core.
2. Install the required libraries.
3. Connect the hardware.
4. Compile and upload.
5. Open the RemoteXY app.
6. Connect via Bluetooth.
7. Perform the 0° / 45° / 90° calibration.

---

## Future Ideas

* Automatic calibration wizard
* Rotary encoder menu system
* Physical calibration buttons
* Battery-powered operation
* Audible angle target indicator
* Temperature monitoring
* Multiple faceting machine profiles
* SD card calibration backup
* Wi-Fi firmware updates
* Web configuration interface

---

## License

MIT License

---

## Acknowledgements

Special thanks to:

* Infineon Technologies for the TLE5012 magnetic angle sensor
* The RemoteXY project
* The gem faceting community for inspiring this project

---

## Project Status

Current Version: **1.0**

The hardware and software are fully functional and suitable for daily use on a faceting machine.

Contributions, bug reports, and suggestions are always welcome.
