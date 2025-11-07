#!/bin/bash

# === Simple build script for SYSC4001_A2_P3 ===
# Creates bin directory and compiles all sources.

if [ ! -d "bin" ]; then
    mkdir bin
else
    rm -f bin/*
fi

# Compile
g++ -g -O0 -std=c++17 -I . -o bin/sim main.cpp Interrupts_101297993_101302793.cpp

echo "Build complete. Run ./bin/sim to start the simulator."
