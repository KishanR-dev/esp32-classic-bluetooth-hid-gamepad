# Custom ESP32 Classic Bluetooth (BR/EDR) HID Gamepad

This repository contains an ESP-IDF implementation of a **Custom ESP32 Classic Bluetooth (BR/EDR) HID Gamepad**.

## Engineering Goals

- **Strict protocol compliance** with Bluetooth Classic HID behavior
- **Accurate HID report mapping** for consistent host-side parsing
- **Broad native compatibility with the Android OS via Bluetooth Classic**

## Technical Scope

The current implementation focuses on:

- NVS initialization
- Bluetooth controller enablement in **Classic BT (BR/EDR)** mode
- Bluedroid initialization and activation
- GAP callback registration and baseline pairing flow handling
- HID Device profile initialization and callback wiring
- Discoverable/connectable scan mode configuration
- Baseline HID report descriptor registration and neutral report transmission

## Project Structure

- `main/main.cpp`  
  Core ESP-IDF application for BR/EDR HID initialization, GAP/HID callbacks, and periodic neutral report transmission.
- `main/CMakeLists.txt`  
  ESP-IDF component registration for the main application source.
- `CMakeLists.txt`  
  Top-level ESP-IDF CMake project definition.
- `sdkconfig.defaults`  
  Default project configuration with Bluetooth Classic-focused options.
- `.gitignore`  
  Excludes generated build and local configuration artifacts from source control.

## Build and Flash (Windows, ESP-IDF PowerShell)

```powershell
# From the project root directory:
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```

Examples for `<PORT>`:
- Windows: `COM5`
- Linux: `/dev/ttyUSB0`
- macOS: `/dev/cu.usbserial-0001`

## Runtime Validation Checklist

1. Verify initialization logs indicate:
   - BT controller enabled in Classic mode
   - Bluedroid enabled
   - HID init event success
   - HID app registration success
2. Confirm Android can discover and pair with the device over Bluetooth Classic.
3. Confirm host recognizes the HID gamepad profile.
4. Validate that input reports are accepted with expected report size and mapping behavior.
5. Iterate HID descriptor and report schema for stronger interoperability where needed.

## Notes

- This codebase targets ESP-IDF and Bluetooth Classic BR/EDR transport.
- Generated files such as `build/`, `sdkconfig`, and `sdkconfig.old` are intentionally excluded from version control.
