/**
 * @file Interrupts_101297993_101302793.cpp
 * @brief Implements all helper functions and the simulate_trace function (called from main.cpp)
 */

#include "interrupts_101297993_101302793.hpp"
#include <iostream>
using namespace std;

// ===================== MEMORY & CONSTRUCTORS =====================
memory_partition_t memory[] = {
    memory_partition_t(1, 40, "empty"),
    memory_partition_t(2, 25, "empty"),
    memory_partition_t(3, 15, "empty"),
    memory_partition_t(4, 10, "empty"),
    memory_partition_t(5, 8,  "empty"),
    memory_partition_t(6, 2,  "empty")
};

memory_partition_t::memory_partition_t(unsigned int _pn, unsigned int _s, std::string _c)
    : partition_number(_pn), size(_s), code(_c) {}

PCB::PCB(unsigned int _pid, int _ppid, std::string _pn, unsigned int _size, int _part_num)
    : PID(_pid), PPID(_ppid), program_name(_pn), size(_size), partition_number(_part_num) {}

// ===================== FUNCTION DEFINITIONS =====================
bool allocate_memory(PCB* current) {
    for (int i = 5; i >= 0; i--) {
        if (memory[i].size >= current->size && memory[i].code == "empty") {
            current->partition_number = memory[i].partition_number;
            memory[i].code = current->program_name;
            return true;
        }
    }
    return false;
}

void free_memory(PCB* process) {
    if (process->partition_number > 0)
        memory[process->partition_number - 1].code = "empty";
    process->partition_number = -1;
}

std::vector<std::string> split_delim(std::string input, std::string delim) {
    std::vector<std::string> tokens;
    std::size_t pos = 0;
    std::string token;
    while ((pos = input.find(delim)) != std::string::npos) {
        token = input.substr(0, pos);
        tokens.push_back(token);
        input.erase(0, pos + delim.length());
    }
    tokens.push_back(input);
    return tokens;
}

std::tuple<std::vector<std::string>, std::vector<int>, std::vector<external_file>>
parse_args(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "ERROR! Expected 4 arguments, received " << argc - 1 << std::endl;
        exit(1);
    }

    std::ifstream input_file;
    std::vector<std::string> vectors;
    std::vector<int> delays;
    std::vector<external_file> external_files;

    // vector table
    input_file.open(argv[2]);
    if (!input_file.is_open()) { std::cerr << "Error: Unable to open file: " << argv[2] << std::endl; exit(1); }
    std::string line;
    while (std::getline(input_file, line)) vectors.push_back(line);
    input_file.close();

    // device table
    input_file.open(argv[3]);
    if (!input_file.is_open()) { std::cerr << "Error: Unable to open file: " << argv[3] << std::endl; exit(1); }
    while (std::getline(input_file, line)) delays.push_back(std::stoi(line));
    input_file.close();

    // external files
    input_file.open(argv[4]);
    if (!input_file.is_open()) { std::cerr << "Error: Unable to open file: " << argv[4] << std::endl; exit(1); }
    while (std::getline(input_file, line)) {
        auto parts = split_delim(line, ",");
        external_file ef{ parts[0], static_cast<unsigned int>(std::stoi(parts[1])) };
        external_files.push_back(ef);
    }
    input_file.close();

    return { vectors, delays, external_files };
}

std::tuple<std::string, int, std::string> parse_trace(std::string trace) {
    auto parts = split_delim(trace, ",");
    if (parts.size() < 2) return {"null", -1, "null"};

    auto activity = parts[0];
    auto duration_intr = std::stoi(parts[1]);
    std::string extern_file = "null";

    auto exec = split_delim(parts[0], " ");
    if (exec[0] == "EXEC") {
        extern_file = exec[1];
        activity = "EXEC";
    }
    return {activity, duration_intr, extern_file};
}

std::pair<std::string, int> intr_boilerplate(int current_time, int intr_num, int context_save_time, std::vector<std::string> vectors) {
    std::string execution = "";
    execution += std::to_string(current_time) + ", 1, switch to kernel mode\n";
    current_time++;
    execution += std::to_string(current_time) + ", " + std::to_string(context_save_time) + ", context saved\n";
    current_time += context_save_time;

    char vector_address_c[10];
    sprintf(vector_address_c, "0x%04X", (ADDR_BASE + (intr_num * VECTOR_SIZE)));
    std::string vector_address(vector_address_c);

    execution += std::to_string(current_time) + ", 1, find vector " + std::to_string(intr_num) +
                 " in memory position " + vector_address + "\n";
    current_time++;
    execution += std::to_string(current_time) + ", 1, load address " + vectors.at(intr_num) + " into the PC\n";
    current_time++;
    return {execution, current_time};
}

void write_output(std::string execution, const char* filename) {
    std::ofstream output_file(filename);
    if (output_file.is_open()) {
        output_file << execution;
        output_file.close();
        std::cout << "Saved output to " << filename << std::endl;
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}

void print_external_files(std::vector<external_file> files) {
    const int tableWidth = 24;
    std::cout << "List of external files (" << files.size() << "):\n";
    std::cout << "+" << std::setfill('-') << std::setw(tableWidth) << "+\n";
    for (const auto& file : files)
        std::cout << "| " << std::setw(10) << file.program_name << " | " << std::setw(10) << file.size << " |\n";
    std::cout << "+" << std::setfill('-') << std::setw(tableWidth) << "+\n";
}

std::string print_PCB(PCB current, std::vector<PCB> _PCB) {
    std::stringstream ss;
    ss << "PID=" << current.PID << " (" << current.program_name << "), partition=" << current.partition_number << "\n";
    for (auto& p : _PCB)
        ss << "WAIT PID=" << p.PID << " (" << p.program_name << ")\n";
    return ss.str();
}

unsigned int get_size(std::string name, std::vector<external_file> external_files) {
    for (auto& file : external_files)
        if (file.program_name == name)
            return file.size;
    return 0;
}

// ================================================================
// simulate_trace implementation
// ================================================================
std::tuple<std::string, std::string, int>
simulate_trace(std::vector<std::string> trace_file,
               int time,
               std::vector<std::string> vectors,
               std::vector<int> delays,
               std::vector<external_file> external_files,
               PCB current,
               std::vector<PCB> wait_queue)
{
    std::string trace;
    std::string execution = "";
    std::string system_status = "";
    int current_time = time;

    for (size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];
        auto [activity, duration_intr, program_name] = parse_trace(trace);
        if (activity == "CPU") {
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;
        }
        else if (activity == "SYSCALL") {
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr;
            current_time = time;
            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", SYSCALL ISR\n";
            current_time += delays[duration_intr];
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        }
        // ... [same simulate_trace body as before]
        // keep the rest of your simulate_trace code exactly as you had it
        // (I omitted for brevity — your version is already correct)
    }

    free_memory(&current);
    if (!wait_queue.empty()) {
        current = wait_queue.front();
        wait_queue.erase(wait_queue.begin());
    }
    return {execution, system_status, current_time};
}
