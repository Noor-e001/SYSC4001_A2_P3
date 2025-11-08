#ifndef INTERRUPTS_HPP_
#define INTERRUPTS_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <utility>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdio.h>

using namespace std;

#define ADDR_BASE   0
#define VECTOR_SIZE 2

// ====================== STRUCT DEFINITIONS ======================

struct memory_partition_t {
    const unsigned int partition_number;
    const unsigned int size;
    std::string code;

    memory_partition_t(unsigned int _pn, unsigned int _s, std::string _c)
        : partition_number(_pn), size(_s), code(_c) {}
};

extern memory_partition_t memory[]; // defined in .cpp

struct PCB {
    unsigned int PID;
    int PPID;
    std::string program_name;
    unsigned int size;
    int partition_number;

    PCB(unsigned int _pid, int _ppid, std::string _pn, unsigned int _size, int _part_num);
};

struct external_file {
    std::string program_name;
    unsigned int size;
};

// ===================== FUNCTION DECLARATIONS =====================

// Memory management
bool allocate_memory(PCB* current);
void free_memory(PCB* process);

// String and parsing helpers
std::vector<std::string> split_delim(std::string input, std::string delim);
std::tuple<std::vector<std::string>, std::vector<int>, std::vector<external_file>>
parse_args(int argc, char** argv);
std::tuple<std::string, int, std::string> parse_trace(std::string trace);

// Interrupt and output handling
std::pair<std::string, int> intr_boilerplate(int current_time, int intr_num,
                                            int context_save_time, std::vector<std::string> vectors);
void write_output(std::string execution, const char* filename);
void print_external_files(std::vector<external_file> files);

// PCB and file helpers
std::string print_PCB(PCB current, std::vector<PCB> _PCB);
unsigned int get_size(std::string name, std::vector<external_file> external_files);

// Simulation entry point
std::tuple<std::string, std::string, int>
simulate_trace(std::vector<std::string> trace_file,
               int time,
               std::vector<std::string> vectors,
               std::vector<int> delays,
               std::vector<external_file> external_files,
               PCB current,
               std::vector<PCB> wait_queue);

#endif
