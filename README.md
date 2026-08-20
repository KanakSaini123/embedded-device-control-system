\# Embedded Device Control and Simulation System



A beginner-friendly embedded software project for monitoring and controlling a simulated energy supply system.



\## Project Overview



The project demonstrates a small task-based embedded control application using C/C++ and FreeRTOS.



The system simulates voltage, current, and temperature measurements and evaluates different operating and fault conditions.



\## System Architecture



The application consists of three FreeRTOS tasks:



\- SensorTask - generates simulated sensor measurements

\- ControlTask - evaluates sensor values and determines the system state

\- StatusTask - reports the current system condition



The controller uses three main states:



\- NORMAL

\- WARNING

\- FAULT



\## Simulation Scenarios



The system currently simulates:



\- Normal operation

\- High temperature

\- Overcurrent

\- Overvoltage

\- Critical combined fault condition



\## Python Test Tool



A Python-based simulation and validation script provides predefined test scenarios and compares expected and calculated system states.



Example result:



```text

Scenario: HIGH TEMPERATURE

Expected: WARNING

Actual: WARNING

Result: PASS

