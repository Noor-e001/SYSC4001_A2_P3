#include "interrupts_101297993_101302793.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <tuple>
#include <algorithm>
#include <utility>

using namespace std;

memory_partition_t memory[] = {
    memory_partition_t(1, 40, "empty"),
    memory_partition_t(2, 25, "empty"),
    memory_partition_t(3, 15, "empty"),
    memory_partition_t(4, 10, "empty"),
    memory_partition_t(5, 8,  "empty"),
    memory_partition_t(6, 2,  "empty")
};

// PCB constructor
PCB::PCB(unsigned int _pid, int _ppid, std::string _pn, unsigned int _size, int _part_num)
    : PID(_pid), PPID(_ppid), program_name(_pn), size(_size), partition_number(_part_num) {}

// Allocate a program to memory
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

// Free memory given a PCB
void free_memory(PCB* process) {
    memory[process->partition_number - 1].code = "empty";
    process->partition_number = -1;
}

// Split helper
vector<string> split_delim(string input, string delim) {
    vector<string> tokens;
    size_t pos = 0;
    while ((pos = input.find(delim)) != string::npos) {
        tokens.push_back(input.substr(0, pos));
        input.erase(0, pos + delim.length());
    }
    tokens.push_back(input);
    return tokens;
}

// Parse configuration files
tuple<vector<string>, vector<int>, vector<external_file>>
parse_args(int argc, char** argv) {
    if (argc != 5) {
        cerr << "ERROR: expected 4 arguments, got " << argc - 1 << endl;
        exit(1);
    }

    ifstream in;
    vector<string> vectors;
    vector<int> delays;
    vector<external_file> external_files;

    // --- Vector table ---
    in.open(argv[2]);
    if (!in.is_open()) { cerr << "Cannot open " << argv[2] << endl; exit(1); }
    string line;
    while (getline(in, line)) vectors.push_back(line);
    in.close();

    // --- Device table (safe stoi) ---
    in.open(argv[3]);
    if (!in.is_open()) { cerr << "Cannot open " << argv[3] << endl; exit(1); }
    while (getline(in, line)) {
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end()); // trim spaces
        if (line.empty()) continue; // skip blank lines
        try {
            vector<string> parts = split_delim(line, ",");
            string secondPart = parts.size() >1 ? parts[1] : line;
            line = secondPart;
            delays.push_back(stoi(line));
        } catch (...) {
            cerr << " Invalid number in device table line: " << line << endl;
        }
    }
    in.close();

    // --- External files ---
    in.open(argv[4]);
    if (!in.is_open()) { cerr << "Cannot open " << argv[4] << endl; exit(1); }
    while (getline(in, line)) {
        auto parts = split_delim(line, ",");
        if (parts.size() == 2) {
            try {
                external_file ef{parts[0], (unsigned)stoi(parts[1])};
                external_files.push_back(ef);
            } catch (...) {
                cerr << "Invalid line in external files table: " << line << endl;
            }
        }
    }
    in.close();

    return {vectors, delays, external_files};
}

// Safer Parse trace line
tuple<string, int, string> parse_trace(string trace) {
    auto parts = split_delim(trace, ",");
    if (parts.size() < 2) {
        cerr << "Skipping malformed trace line: " << trace << endl;
        return {"null", -1, "null"};
    }

    // Trim whitespace
    auto trim = [](string &s) {
        s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
    };
    trim(parts[0]);
    trim(parts[1]);

    string activity = parts[0];
    int val = 0;
    try {
        val = stoi(parts[1]);
    } catch (...) {
        cerr << "Invalid number in trace line: " << trace << endl;
        val = 0;
    }

    string extra = "null";
    auto exec = split_delim(parts[0], " ");
    if (exec[0] == "EXEC" && exec.size() > 1)
        extra = exec[1];

    return {activity, val, extra};
}

// Interrupt boilerplate
pair<string, int> intr_boilerplate(int current_time, int intr_num, int context_save_time, vector<string> vectors) {
    string out;
    out += to_string(current_time) + ",1,switch to kernel mode\n"; current_time++;
    out += to_string(current_time) + "," + to_string(context_save_time) + ",context saved\n";
    current_time += context_save_time;

    char buf[16];
    sprintf(buf, "0x%04X", (ADDR_BASE + (intr_num * VECTOR_SIZE)));
    string addr(buf);

    out += to_string(current_time) + ",1,find vector " + to_string(intr_num) + " in " + addr + "\n"; current_time++;
    out += to_string(current_time) + ",1,load " + vectors[intr_num] + " into PC\n"; current_time++;

    return {out, current_time};
}

// Write to file
void write_output(string content, const char* filename) {
    ofstream out(filename);
    if (out.is_open()) out << content;
    out.close();
}

// Print external file table
void print_external_files(vector<external_file> files) {
    cout << "External files:\n";
    for (auto &f : files)
        cout << "  " << f.program_name << " (" << f.size << " KB)\n";
}

// Print PCB table (formatted box style)
string print_PCB(PCB current, vector<PCB> list) {
    stringstream ss;
    const int width = 60;

    ss << "+-----------------------------------------------------------+\n";
    ss << "| PID | Program Name | Partition # | Size |     State      |\n";
    ss << "+-----------------------------------------------------------+\n";

    // Current (running) process
    ss << "| " << setw(3) << current.PID
       << " | " << setw(12) << left << current.program_name
       << " | " << setw(12) << right << current.partition_number
       << " | " << setw(4) << right << current.size
       << " | " << setw(12) << left << "running" << " |\n";

    // Waiting processes
    for (auto &p : list) {
        ss << "| " << setw(3) << p.PID
           << " | " << setw(12) << left << p.program_name
           << " | " << setw(12) << right << p.partition_number
           << " | " << setw(4) << right << p.size
           << " | " << setw(12) << left << "waiting" << " |\n";
    }

    ss << "+-----------------------------------------------------------+\n";
    return ss.str();
}

// Get size of program
unsigned int get_size(string name, vector<external_file> files) {
    for (auto &f : files)
        if (f.program_name == name) return f.size;
    return 0;
}

// Simulate trace
tuple<string, string, int> simulate_trace(vector<string> trace_file, int time,
                                          vector<string> vectors, vector<int> delays,
                                          vector<external_file> ext_files, PCB current,
                                          vector<PCB> wait_queue) {
    string exec_log;
    string sys_log;
    int context_save_time = 2;

    for (auto &line : trace_file) {
        auto [activity, val, extra] = parse_trace(line);

        if (activity == "FORK") {
            auto [log, new_time] = intr_boilerplate(time, 0, context_save_time, vectors);
            exec_log += log;
            time = new_time;
        }
        else if (activity == "EXEC") {
            unsigned sz = get_size(extra, ext_files);
            PCB newpcb(current.PID + 1, current.PID, extra, sz, -1);
            if (allocate_memory(&newpcb)) {
                exec_log += to_string(time) + ", " + to_string(val) + ", executing " + extra + "\n";
                time += val;
                free_memory(&newpcb);
            }
        }
        else if (activity == "IF_CHILD" || activity == "IF_PARENT" || activity == "ENDIF") {
            exec_log += to_string(time) + ",1," + activity + "\n";
            time++;
        }
    }

    sys_log = print_PCB(current, wait_queue);
    return {exec_log, sys_log, time};
}
