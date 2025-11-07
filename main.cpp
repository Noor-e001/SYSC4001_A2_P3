#include "interrupts_101297993_101302793.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
using namespace std;
namespace fs = std::filesystem;

int main() {
    vector<string> traceFiles = {
        "input_files/trace_1.txt",
        "input_files/trace_2.txt",
        "input_files/trace_3.txt",
        "input_files/trace_4.txt",
        "input_files/trace_5.txt"
    };

    string inputDir = "input_files";
    string outputDir = "output_files";

    if (!fs::exists(outputDir))
        fs::create_directory(outputDir);

    int simIndex = 1;

    for (const auto& tracePath : traceFiles) {
        if (!fs::exists(tracePath)) {
            cerr << "[Skip] " << tracePath << " not found.\n";
            continue;
        }

        cout << "\n=== Running Simulation " << simIndex
             << " (" << tracePath << ") ===\n";

        // Define configuration files
        string vecFile = inputDir + "/vector_table.txt";
        string devFile = inputDir + "/device_table.txt";
        string extFile = inputDir + "/external_files.txt";

        // ✅ Proper C++ way to simulate argv array
        std::vector<char*> argvVec;
        argvVec.push_back((char*)"sim");
        argvVec.push_back((char*)tracePath.c_str());
        argvVec.push_back((char*)vecFile.c_str());
        argvVec.push_back((char*)devFile.c_str());
        argvVec.push_back((char*)extFile.c_str());

        // Parse configuration files
        auto [vectors, delays, external_files] =
            parse_args(static_cast<int>(argvVec.size()), argvVec.data());

        // Read trace file
        ifstream traceInput(tracePath);
        if (!traceInput.is_open()) {
            cerr << "Error: could not open " << tracePath << endl;
            continue;
        }

        PCB current(0, -1, "init", 1, -1);
        if (!allocate_memory(&current)) {
            cerr << "ERROR! Memory allocation failed!\n";
            continue;
        }

        vector<PCB> wait_queue;
        vector<string> trace_lines;
        string line;
        while (getline(traceInput, line)) {
            if (!line.empty()) trace_lines.push_back(line);
        }
        traceInput.close();

        // ✅ Run the simulation
        auto [execution, system_status, _] =
            simulate_trace(trace_lines, 0, vectors, delays, external_files, current, wait_queue);

        // --- Save output files for each trace run ---
        string execOut = outputDir + "/execution_" + to_string(simIndex) + ".txt";
        string sysOut = outputDir + "/system_status_" + to_string(simIndex) + ".txt";

        write_output(execution, execOut.c_str());
        write_output(system_status, sysOut.c_str());

        cout << "Saved logs:\n  " << execOut << "\n  " << sysOut << "\n";
        simIndex++;
    }

    cout << "\n All simulations complete. Check output_files/ for results.\n";
    return 0;
}
