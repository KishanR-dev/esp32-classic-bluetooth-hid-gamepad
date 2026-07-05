# Custom ESP32 Classic Bluetooth (BR/EDR) HID Gamepad

This repository contains an ESP-IDF implementation of a **Custom ESP32 Classic Bluetooth (BR/EDR) HID Gamepad**.

## Engineering Goals

- **Strict protocol compliance** with Bluetooth Classic HID behavior
- **Accurate HID report mapping** for consistent host-side parsing
- **Broad native compatibility with the Android OS via Bluetooth Classic**

## Features Implemented

- NVS initialization
- Bluetooth controller enablement in **Classic BT (BR/EDR)** mode
- Bluedroid initialization and activation
- GAP callback registration and baseline pairing flow handling
- HID Device profile initialization and callback wiring
- Discoverable/connectable scan mode configuration
- Baseline HID report descriptor registration and neutral report transmission loop

## Required Libraries and Tooling

This project uses **ESP-IDF** and its bundled components. No third-party Arduino libraries are required for the ESP-IDF target in `esp32-classic-hid/`.

### Core requirements

- ESP32 development board (classic ESP32 target)
- USB data cable
- Python 3.8+ (installed by ESP-IDF tools installer in most setups)
- Git
- ESP-IDF (recommended stable release, e.g. v5.x or v6.x with ESP32 support)

### ESP-IDF components used (from ESP-IDF SDK)

- Bluetooth controller / Bluedroid host stack
- HID device APIs
- FreeRTOS (task delay / runtime loop)
- NVS flash
- ESP system logging/error handling

## Installation Guide

### 1) Install ESP-IDF

Follow official Espressif setup instructions for your platform:

- Windows: ESP-IDF Tools Installer (recommended)
- Linux/macOS: ESP-IDF manual installation with export script

Official docs:
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

### 2) Clone the repository

```bash
git clone https://github.com/KishanR-dev/esp32-classic-bluetooth-hid-gamepad.git
cd esp32-classic-bluetooth-hid-gamepad/esp32-classic-hid
```

### 3) Open an ESP-IDF-enabled shell

- Windows: **ESP-IDF PowerShell**
- Linux/macOS: shell where `export.sh` has been sourced

Verify setup:

```bash
idf.py --version
python --version
```

### 4) Build firmware

```bash
idf.py set-target esp32
idf.py build
```

### 5) Flash and monitor

```bash
idf.py -p <PORT> flash monitor
```

Examples for `<PORT>`:
- Windows: `COM5`
- Linux: `/dev/ttyUSB0`
- macOS: `/dev/cu.usbserial-0001`

## Runtime Verification

1. Confirm logs indicate:
   - BT controller enabled in Classic mode
   - Bluedroid enabled
   - HID init success
   - HID app registration success
2. Confirm device discovery and pairing on Android over Bluetooth Classic.
3. Confirm host recognizes HID gamepad profile.
4. Confirm input reports are received with expected mapping behavior.

## Troubleshooting

- **`idf.py` not recognized**
  - Use ESP-IDF shell or source ESP-IDF export script.
- **Serial port not found**
  - Check USB cable (data-capable), drivers, and port permissions.
- **Flash connection issues**
  - Hold BOOT while starting flash on some boards, then release once flashing starts.
- **No Bluetooth visibility**
  - Confirm successful boot logs and scan mode setup in monitor output.

## Project Structure

- `main/main.cpp`  
  Core ESP-IDF application for BR/EDR HID initialization, GAP/HID callbacks, and neutral report transmission.
- `main/CMakeLists.txt`  
  ESP-IDF component registration for main source.
- `CMakeLists.txt`  
  Top-level ESP-IDF CMake project definition.
- `sdkconfig.defaults`  
  Default project configuration with Bluetooth Classic-focused options.
- `.gitignore`  
  Excludes generated and local machine configuration artifacts from version control.

## Notes

- Generated files such as `build/`, `sdkconfig`, and `sdkconfig.old` are intentionally excluded from Git.
- If you update ESP-IDF major versions, re-run `idf.py menuconfig` and validate symbol compatibility in `sdkconfig.defaults`.
