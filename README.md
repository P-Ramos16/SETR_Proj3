# SETR_Proj3 - Temperature Control System

Repository for the third project of the 24-25 SETR class at Universidade de Aveiro
A Zephyr RTOS-based temperature regulation system with PID control, UART command interface, and LED status indicators.

## Features

- Real-time temperature monitoring via I2C (TC74 sensor)
- PID controller for precise temperature regulation
- Heater control via FET
- UART command interface for system control
- LED status indicators
- Button controls for manual operation
- Thread-safe RTDB (Real-Time Database) for data sharing

## Thread Architecture

<img src="../architecture.png" alt="Thread Architecture" width="600"/>

## Threads

1. **LED Update Task**: Updates LED indicators based on system state
2. **Temperature Reading Task**: Reads temperature from sensor
3. **PID Controller Task**: Calculates PID output
4. **Heat Control Task**: Controls heater based on PID output
5. **UART Command Task**: Processes incoming UART commands

## UART Commands

| Command | Format | Description |
|---------|--------|-------------|
| Get Current Temp | `#C067!` | Returns current temperature |
| Get Desired Temp | `#D068!` | Returns desired temperature |
| Set Desired Temp | `#M+30219!` | Sets desired temperature (+30.2°C) |
| Set PID Params | `#Sp1.23135!` | Sets PID parameters (P=1.23, i and d options are also available) |
| Toggle Verbose | `#V086!` | Toggles verbose mode |

## File Structure
```
.
├── build/
├── docs/
├── examples/
│
├── nrf52840dk_nrf52840.overlay
├── prj.conf
├── README.md
├── CMakeLists.txt
├── Doxyfile
│
├── src
│   ├── main.c
│   └── modules
│       ├── buttons.c
│       ├── buttons.h
│       ├── CMakeLists.txt
│       ├── cmdproc.c
│       ├── cmdproc.h
│       ├── PID.c
│       ├── PID.h
│       ├── rtdb.c
│       └── rtdb.h
│
└── tests
    ├── build
    ├── Unity
    ├── modules
    │   ├── CMakeLists.txt
    │   ├── cmdproc.c
    │   ├── cmdproc.h
    │   ├── PID.c
    │   ├── PID.h
    │   ├── rtdb.c
    │   └── rtdb.h
    ├── PID_tests.c
    ├── cmdproc_tests.c
    └── CMakeLists.txt
```