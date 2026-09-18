# LPC2148 Contactless Liquid Level Controller

A contactless water level monitoring and automatic pump control system developed using the **NXP LPC2148 microcontroller** and **HC-SR04 ultrasonic sensor**.

## Description

The Contactless Liquid Level Controller is an intelligent water management system designed to monitor the height of water in a tank without direct contact with the liquid.

The **HC-SR04 ultrasonic sensor** measures the distance between the sensor and the water surface. The **LPC2148 microcontroller** processes this measurement, calculates the water level, and automatically controls a DC motor (water pump).

The current water level and pump status are displayed on a **16×2 I2C LCD**.

## Features

- Contactless water level measurement
- Continuous water level monitoring
- Automatic water pump control
- Tank overflow prevention
- Protection against dry-running conditions
- Real-time water level display
- Pump status indication on LCD

## Hardware Components

- NXP LPC2148 Microcontroller
- HC-SR04 Ultrasonic Sensor
- DC Motor / Water Pump
- L293D Motor Driver IC
- 16×2 LCD Display
- I2C Interface
- Power Supply Circuit

## Working Principle

1. The HC-SR04 ultrasonic sensor is positioned at the top of the water tank.
2. The sensor measures the distance between itself and the water surface.
3. The LPC2148 calculates the corresponding water level from the measured distance.
4. When the water level is below the maximum level, the DC motor is switched ON.
5. The motor pumps water into the tank.
6. When the water level reaches 100%, the motor is switched OFF.
7. The LCD displays the water level, measured distance, and motor status.

## Water Level Calculation

The system uses an approximate distance range of **10 cm to 30 cm**.

| Distance | Water Level |
|----------|-------------|
| ≥ 30 cm  | 0% |
| 25 cm    | 25% |
| 20 cm    | 50% |
| 15 cm    | 75% |
| ≤ 10 cm  | 100% |

## Pin Connections

| Component | LPC2148 Pin |
|-----------|-------------|
| HC-SR04 TRIG | P0.5 |
| HC-SR04 ECHO | P0.4 |
| Pump/Relay Control | P0.11 |

The LCD communicates with the LPC2148 through the I2C interface.

## Software Requirements

- Embedded C
- Keil µVision
- NXP LPC2148

## Getting Started

### Prerequisites

The following tools are required to work with the project:

- LPC2148 development environment
- Keil µVision
- Embedded C compiler
- Proteus, if the project is simulated

### Running the Project

1. Open the project in **Keil µVision**.
2. Add the project source code.
3. Configure the LPC2148 target.
4. Compile the program.
5. Generate the required output file.
6. Load the program into the LPC2148 or use it with the Proteus simulation.
7. Connect the HC-SR04, LCD, and pump control circuit according to the circuit design.
