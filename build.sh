if [ ! -d "bin" ]; then
    mkdir bin
else
    rm -f bin/*
fi

g++ -g -O0 -I . -o bin/sim main.cpp Interrupts_101297993_101302793.cpp
