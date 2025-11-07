# SYSC4001 Assignment 2 : Part 3: Simulation Build and Results

## Overview
This project implements a simplified simulation of an interrupt driven execution environment for **SYSC4001: Operating Systems**.  
It demonstrates the interaction between process creation, interrupt handling, and execution scheduling.

The project was developed, built, and executed under **WSL Ubuntu 24.04** using **CLion** with a CMake-based build system.

---

## Team Members
- **Student 1:** Noor E (101297993)  
- **Student 2:** Lina Elsayed (101302793)

---

## Directory Structure
SYSC4001_A2_P3/
│
├── CMakeLists.txt
├── main.cpp
├── Interrupts_101297993_101302793.cpp
├── interrupts_101297993_101302793.hpp
│
├── input_files/
│ ├── device_table.txt
│ ├── external_files.txt
│ ├── trace.txt
│ └── vector_table.txt
│
├── output_files/
│ ├── execution.txt
│ └── system_status.txt
│
└── build/ (auto generated)

---

## Build Instructions
1. Open the project in **CLion** under your WSL environment.
2. Ensure the WSL toolchain is active: 'Ubuntu-24.04', 'gcc/g++', 'gdb', 'cmake'
3. Build the project using:
   "bash"
   mkdir -p build && cd build
   cmake ..
   make

## Run the Simulation
./cmake-build-debug/sim

## Output
Simulation results are generated under the output_files/ directory:
execution.txt: shows the chronological execution events (FORK, EXEC)
system_status.txt: displays system states at each timestamp

Example:

time: 10; FORK
1 init 6 1 running
0 init 6 1 running

time: 60; EXEC program1
1 init 6 1 running
0 init 6 1 running

Tools and Environment

Operating System: Ubuntu 24.04 (WSL)

Compiler: GCC / G++

Debugger: GDB

IDE: CLion

Build System: CMake 3.28.3

GitHub Repository

The full project source, inputs, and generated outputs are available at:
https://github.com/Noor-e001/SYSC4001_A2_P3
