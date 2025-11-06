#include "interrupts_101297993_101302793.hpp"
#include <iostream>

int main() {
    try {
        Simulator sim("input_files", "output_files");
        sim.loadAll();
        sim.run();
        std::cout << "Simulation complete. Check output_files/ for logs.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
