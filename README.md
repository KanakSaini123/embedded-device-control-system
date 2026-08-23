# Embedded Device Control and Simulation System

A beginner-friendly embedded software project that demonstrates a task-based control system using **C/C++**, **FreeRTOS**, and a **Python-based simulation and testing environment**.

The system simulates an energy supply device by processing voltage, current, and temperature measurements and determining the current operating state.

## Project Overview

The project consists of a C/C++ application running on the **FreeRTOS Windows simulator** and a Python simulator/test tool.

The Python application sends simulated operating and fault conditions to the C++ application using **TCP and JSON**.

The C++ application receives the measurements, processes them using FreeRTOS tasks, determines the system state, and sends the result back to Python.

```text
Python Simulator
      |
      | JSON over TCP
      v
+---------------------------+
| C/C++ FreeRTOS           |
|                           |
|  SensorTask               |
|  ControlTask              |
|  StatusTask               |
|  NetworkTask              |
|                           |
|  SensorData               |
|  SystemState              |
+---------------------------+
      |
      | JSON over TCP
      v
Python Test Runner
      |
      v
Expected vs Actual
      |
      v
PASS / FAIL