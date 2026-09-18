# 💧 Contactless Liquid Level Controller using LPC2148

> Smart Water Level Indicator with Pump Control — an ARM7 (LPC2148) based system that measures tank water level **without contact** using an HC-SR04 ultrasonic sensor and automatically controls the water pump to prevent **overflow** and **dry-running**.

![Platform](https://img.shields.io/badge/MCU-NXP%20LPC2148%20(ARM7)-blue)
![Language](https://img.shields.io/badge/language-Embedded%20C-green)

---

## 📋 Table of Contents
- [Overview](#-overview)
- [Objectives](#-objectives)
- [Hardware & Components](#-hardware--components)
- [Pin Connections](#-pin-connections)
- [How It Works](#-how-it-works)
- [Control Logic](#-control-logic)
- [Signal Processing (Software Filters)](#-signal-processing-software-filters)
- [LCD Display Output](#-lcd-display-output)
- [Installation & Usage](#-installation--usage)
- [Results](#-results)
- [Conclusion](#-conclusion)
- [Team](#-team)

---

## 🔎 Overview

The **Contactless Liquid Level Controller** is an intelligent water management system developed using the **NXP LPC2148** microcontroller (ARM7 core, 12 MHz crystal).

- An **HC-SR04 Ultrasonic Sensor** mounted at the top of the tank continuously measures the distance to the water surface — no probe ever touches the liquid.
- Based on the measured distance, the LPC2148 computes the water level as a **percentage** and automatically switches the **DC Motor (Water Pump)** ON or OFF.
- The motor starts when the water level drops below the minimum limit and stops when the tank becomes full.
- A **16×2 I2C LCD** displays the live water level and pump status.

This automation saves water, reduces manual monitoring, and improves the efficiency of the water storage system.

## 🎯 Objectives

1. Monitor the water level continuously.
2. Measure liquid level **without physical contact**.
3. Automatically operate the DC Motor for tank filling.
4. Avoid water overflow conditions.
5. Protect the pump from **dry-running**.
6. Display water level and motor status on LCD.
7. Improve water utilization and system reliability.

## 🧰 Hardware & Components

| # | Component | Role |
|---|-----------|------|
| 1 | **NXP LPC2148 Microcontroller** | ARM7 (ARM7TDMI) core @ 12 MHz crystal — main controller |
| 2 | **Ultrasonic Sensor (HC-SR04)** | Contactless distance / level measurement |
| 3 | **DC Motor (Water Pump)** | Fills the tank |
| 4 | **L293D Motor Driver IC** | Drives the pump motor from the controller output |
| 5 | **16×2 LCD Display (I2C)** | Live level + pump status readout |
| 6 | **Power Supply Circuit** | Regulated supply for MCU, sensor and motor |

## 🔌 Pin Connections

As defined in [`water_level_controller.c`](water_level_controller.c):

| LPC2148 Pin | Connected To | Direction | Notes |
|-------------|--------------|-----------|-------|
| **P0.5** | HC-SR04 `TRIG` | Output | 10 µs trigger pulse |
| **P0.4** | HC-SR04 `ECHO` | Input | Echo pulse width measured with Timer0 |
| **P0.11** | Relay / Motor driver input (pump) | Output | `IOSET0` = pump ON, `IOCLR0` = pump OFF |
| **P0.2 (SDA0) / P0.3 (SCL0)** | 16×2 I2C LCD (PCF8574 backpack, address `0x4E`) | I2C0 | Configured via `PINSEL0 = 0x50` |

**Timing configuration:** Timer0 is prescaled with `T0PR = 11` so one timer tick = **1 µs** at the 12 MHz clock. Echo pulse width (µs) ÷ 58 = distance in **cm**; readings are abandoned above 38000 µs (timeout).

## ⚙️ How It Works

1. The ultrasonic sensor is fixed at the **top of the water tank**.
2. It continuously measures the distance between the sensor and the water surface.
3. The LPC2148 calculates the current water level from the measured distance.
4. If the water level falls below the preset **minimum level**, the controller switches **ON** the DC Motor.
5. The motor pumps water into the tank until the **maximum level** is reached.
6. Once the tank is full, the controller switches **OFF** the motor automatically.
7. The LCD continuously displays the water level and motor condition.

```
       ┌──────────────┐  TRIG (P0.5)   ┌─────────────┐
       │              │───────────────▶│  HC-SR04    │
       │              │◀───────────────│ Ultrasonic  │
       │   LPC2148    │  ECHO (P0.4)   └─────────────┘
       │   (ARM7)     │
       │              │  P0.11         ┌─────────────┐
       │              │───────────────▶│ Relay +     │──▶ DC Water Pump
       │              │                │ L293D Drive │
       │              │  I2C0 (P0.2/3) └─────────────┘
       │              │◀──────────────▶┌─────────────┐
       └──────────────┘                │ 16×2 LCD    │
                                       └─────────────┘
```

## 🧠 Control Logic

The tank is mapped linearly from distance to percentage:

| Distance to water surface | Water Level | Pump Action |
|---------------------------|-------------|-------------|
| **≥ 30 cm** (tank empty) | **0 %** | Pump **ON** (refill) |
| Between 10 cm and 30 cm | `(30 − distance) × 5` % (e.g. 20 cm → 50 %) | Pump **ON** |
| **≤ 10 cm** (tank full) | **100 %** | Pump **OFF** (no overflow) |

- Pump runs from **0 % to 99 %** and switches off only at **100 %**, guaranteeing a full tank while preventing overflow.
- Sensor failsafe: if all 5 samples fail, the distance defaults to **40 cm → 0 % (tank empty)**.

## 🧪 Signal Processing (Software Filters)

The firmware uses robust measurement techniques found in [`water_level_controller.c`](water_level_controller.c):

- **Median filter:** 5 ultrasonic pings are taken per cycle (60 ms apart), sorted, and the **median** value is used — rejecting outliers from ripples, foam and noise. Invalid readings (0 cm or > 50 cm) are discarded before sorting.
- **Hardware timeouts:** every echo wait is bounded (counter-based timeouts; echo measurement capped at `T0TC > 38000` µs) so a missing echo can never hang the system.
- **Anti-flicker LCD:** the display is rewritten **only when the distance or pump state actually changes**, avoiding visible flicker.

## 🖥️ LCD Display Output (16×2)

```
┌─────────────────┐
│ Water Lvl: 90%  │   ← water level percentage
│ D:10cm Pump:OFF │   ← live distance in cm + pump state
└─────────────────┘
```

| Condition | Display |
|-----------|---------|
| Water level 90 % | Valve/Pump **ON** |
| Water level 0 % | Valve/Pump **ON** |
| Water level 100 % | Valve/Pump **OFF** |

## 🧩 Firmware Module Map

All modules live in [`water_level_controller.c`](water_level_controller.c):

| Function(s) | Purpose |
|-------------|---------|
| `main()` | Sensor polling loop, median filter, level computation, pump control, LCD update |
| `initTimer0()` | Timer0 config for 1 µs echo timing (`T0PR = 11`) |
| `i2c_init / i2c_start / i2c_write / i2c_stop` | Hardware I2C0 bit-level driver |
| `lcd_send_nibble / lcd_cmd / lcd_data / lcd_init / lcd_string` | 4-bit I2C LCD driver (address `0x4E`) |
| `delayUS / delayMS` | Software delay loops |

## 🚀 Installation & Usage

The firmware is **bare-metal Embedded C** targeting the LPC2148 and uses the standard `lpc214x.h` register header.

1. **Hardware assembly** — wire the HC-SR04, relay/motor-driver (L293D) and I2C LCD to the LPC2148 as listed in [Pin Connections](#-pin-connections).
2. **Build** — open `water_level_controller.c` in an LPC2148-capable toolchain (e.g. Keil µVision MDK-ARM or GNU Arm Embedded toolchain with `lpc214x.h`), include the register header, and compile for the LPC2148 target.
3. **Flash** — program the resulting binary onto the LPC2148 (e.g. via UART0 ISP boot-loader or your debug probe).
4. **Run** — power up the system:
   - The pump starts automatically whenever the tank is below 100 %.
   - The LCD shows `Water Lvl:XX%` and `D:XXcm Pump:ON/OFF` in real time.
   - When the tank reaches 100 % (distance ≤ 10 cm), the pump switches off automatically.

> ⚠️ **Note:** keep the sensor mounted level at the top of the tank; the 10–30 cm band defines the measurable 0–100 % range (a 20 cm tall measurement band in this configuration).

## 📈 Results

1. Accurate **contactless** water level monitoring.
2. Automatic control of the DC Motor.
3. Prevention of tank overflow.
4. Protection against dry-running.
5. Real-time level indication on LCD.
6. Reduced manual intervention.

## ✅ Conclusion

The LPC2148-based Contactless Liquid Level Controller provides a simple and efficient solution for automatic tank water management. By combining an ultrasonic sensor with a DC motor control mechanism, the system ensures reliable operation, minimizes water wastage, and enhances overall convenience in domestic and industrial applications.

## 👥 Team

**Department of Electrical & Electronics Engineering**

| Name | Division |
|------|----------|
| PRIYA H V | B |
| SHREYA H | B |
| SAYEDA YUSRA | B |
| JYOTI | B |
