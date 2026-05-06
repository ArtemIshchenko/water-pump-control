# Water Pump Controller (ESP32)

An embedded firmware for automated water pump control based on the ESP32 microcontroller, built with PlatformIO and the Arduino framework.

## Overview

This project implements a reliable water pump controller with multiple control sources, automatic safety shutoff, and deep sleep power management. It is designed for real-world use cases such as filling baths or tanks where over-filling prevention is critical.

## Features

- **Multi-source control** — pump can be toggled via a physical button or UART commands from an external display
- **3-channel water level monitoring** — automatic pump shutoff when any sensor detects maximum fill level
- **Safety timer** — pump automatically turns off after 15 minutes of continuous operation (TOF timer)
- **Deep sleep mode** — controller enters deep sleep after 15 minutes of inactivity to save power; wakes on physical button press via EXT0 interrupt
- **Real-time status reporting** — pump state, water level, and selected bath index are broadcast to the display over UART

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 DOIT DevKit V1 |
| Pump output | GPIO 13 |
| Manual button | GPIO 12 (INPUT_PULLUP) |
| Water level sensors | GPIO 14, 27, 26 (INPUT_PULLUP) |
| Display UART | TX: GPIO 17, RX: GPIO 16 |

## Custom Libraries

- **EdgeDetector** — detects rising and falling edges on digital inputs, preventing repeated triggering on held signals
- **TON / TOF / TP** — IEC 61131-3 style software timers (on-delay, off-delay, pulse) for structured timing logic

## Communication Protocol

The controller communicates with an external display module over UART at 115200 baud.

**Incoming commands (display → controller):**
```
CMD:ON   — turn pump on
CMD:OFF  — turn pump off
```

**Outgoing status (controller → display):**
```
PUMP:<0|1>,WATER:<0|1>,BATHS:<n>
```

## Build & Flash

Requires [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Flash
pio run --target upload

# Monitor serial output
pio device monitor
```

## Tech Stack

`C++` · `ESP32` · `Arduino framework` · `PlatformIO` · `FreeRTOS (ESP-IDF base)` · `ESP deep sleep (ext0 wakeup)`
