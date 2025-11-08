## SYSC4001 Assignment 2 — Part 3: Simulation Build and Results
Overview
This project implements a simplified simulation of an interrupt-driven execution environment for SYSC4001: Operating Systems.
It demonstrates the interaction between process creation (FORK), interrupt handling, and execution scheduling (EXEC, SYSCALL, END_IO).
The simulation outputs both execution traces and system status snapshots for multiple input cases.

The project was developed, built, and tested under WSL Ubuntu 24.04 using CLion with a CMake-based build system.

## Team Members
Student 1: Noor E (101297993)
Student 2: Lina Elsayed (101302793)

Directory Structure
SYSC4001_A2_P3/
│
├── CMakeLists.txt
├── main.cpp
├── Interrupts_101297993_101302793.cpp
├── interrupts_101297993_101302793.hpp
│
├── input_files/
│   ├── device_table.txt
│   ├── external_files.txt
│   ├── vector_table.txt
│   ├── trace_1.txt
│   ├── trace_2.txt
│   ├── trace_3.txt
│   ├── trace_4.txt
│   └── trace_5.txt
│
├── output_files/
│   ├── execution_1.txt
│   ├── system_status_1.txt
│   ├── execution_2.txt
│   ├── system_status_2.txt
│   ├── execution_3.txt
│   ├── system_status_3.txt
│   ├── execution_4.txt
│   ├── system_status_4.txt
│   ├── execution_5.txt
│   └── system_status_5.txt
│
└── build/     

## Build Instructions:
Open the project in CLion under your WSL (Ubuntu 24.04) environment.
Ensure your toolchain is correctly configured:
Toolchain: Ubuntu 24.04 (WSL)
Compiler: GCC / G++
Debugger: GDB
Build system: CMake 3.28+

## Output Description

Each simulation generates two output files:

execution_X.txt: chronological log of execution events
(interrupts, forks, context switches, system calls)

system_status_X.txt: system state snapshots showing all processes
(PID, program name, partition number, size, and running/waiting state)

Example (simplified):

time: 24; current trace: FORK, 10
+------------------------------------------------------+
| PID |program name |partition number | size |   state |
+------------------------------------------------------+
|   1 |init         |               5 |    1 |running  |
|   0 |init         |               6 |    1 |waiting  |
+------------------------------------------------------+

GitHub Repository: https://github.com/Noor-e001/SYSC4001_A2_P3 

The full project source code, input configurations, and generated outputs are available at:
🔗 https://github.com/Noor-e001/SYSC4001_A2_P3
