/**
 * @file Interrupts_101297993_101302793.cpp
 * @brief Implements helper functions defined in interrupts_101297993_101302793.hpp
 *        and the simulate_trace() used by main.cpp
 */

#include "interrupts_101297993_101302793.hpp"
#include <iostream>
#include <cctype>
#include <algorithm>

// ------------------------------
// local helpers (file-private)
// ------------------------------
namespace {

/** trim from left */
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            [](unsigned char ch){ return !std::isspace(ch); }));
}

/** trim from right */
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
}

/** trim both */
inline void trim(std::string &s) { ltrim(s); rtrim(s); }

/** split on a single char delimiter, keep empty fields if any */
std::vector<std::string> split_comma(const std::string &line) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : line) {
        if (c == ',') {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

} // namespace

// ======================================================================
// Robust parse_trace()  (DEFINITION lives in this .cpp; only DECLARE in .hpp)
// ======================================================================
std::tuple<std::string, int, std::string> parse_trace(std::string traceLine) {
    // Ignore empty or whitespace-only lines
    std::string raw = traceLine;
    trim(raw);
    if (raw.empty()) return {"null", -1, "null"};

    // Split by comma safely, then trim each token
    auto parts = split_comma(raw);
    for (auto &p : parts) trim(p);

    if (parts.size() < 2) {
        // minimal: ACTIVITY, NUMBER  (e.g., "CPU, 10")
        // If malformed, signal to caller to skip
        return {"null", -1, "null"};
    }

    std::string activity = parts[0];   // e.g. "CPU" or "EXEC program1"
    std::string numToken = parts[1];   // e.g. "10"
    std::string program  = "null";

    // Detect EXEC with program name embedded in the activity token
    // Accept both "EXEC program1" and "EXEC   program1"
    {
        // split activity by spaces
        std::vector<std::string> toks;
        std::string cur;
        for (char c : activity) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!cur.empty()) { toks.push_back(cur); cur.clear(); }
            } else cur.push_back(c);
        }
        if (!cur.empty()) toks.push_back(cur);

        if (!toks.empty() && toks[0] == "EXEC") {
            activity = "EXEC";
            if (toks.size() >= 2) program = toks[1];
        }
    }

    // Safely parse the number (duration / intr #)
    int value = -1;
    try {
        trim(numToken);
        // Allow tokens like "10  " or "  10"
        value = std::stoi(numToken);
    } catch (...) {
        // If non-numeric, tell caller to skip line
        return {"null", -1, "null"};
    }

    return {activity, value, program};
}

// ======================================================================
// simulate_trace()  (DEFINITION lives in this .cpp; only DECLARE in .hpp)
// ======================================================================
std::tuple<std::string, std::string, int>
simulate_trace(std::vector<std::string> trace_file,
               int time,
               std::vector<std::string> vectors,
               std::vector<int> delays,
               std::vector<external_file> external_files,
               PCB current,
               std::vector<PCB> wait_queue)
{
    std::string execution;
    std::string system_status;
    int current_time = time;

    for (size_t i = 0; i < trace_file.size(); i++) {
        const std::string &raw = trace_file[i];
        auto [activity, duration_intr, program_name] = parse_trace(raw);

        // Skip malformed/blank lines
        if (activity == "null" || duration_intr < 0) continue;

        if (activity == "CPU") {
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;
        }
        else if (activity == "SYSCALL") {
            auto [intr, t] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr; current_time = t;

            if (duration_intr >= 0 && duration_intr < static_cast<int>(delays.size()))
                execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", SYSCALL ISR\n";
            else
                execution += std::to_string(current_time) + ", 0, SYSCALL ISR (unknown device)\n";

            current_time += (duration_intr >= 0 && duration_intr < static_cast<int>(delays.size())) ? delays[duration_intr] : 0;
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        }
        else if (activity == "END_IO") {
            auto [intr, t] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr; current_time = t;

            if (duration_intr >= 0 && duration_intr < static_cast<int>(delays.size()))
                execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", ENDIO ISR\n";
            else
                execution += std::to_string(current_time) + ", 0, ENDIO ISR (unknown device)\n";

            current_time += (duration_intr >= 0 && duration_intr < static_cast<int>(delays.size())) ? delays[duration_intr] : 0;
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        }
        else if (activity == "FORK") {
            auto [intr, t] = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr; current_time = t;

            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", cloning the PCB\n";
            current_time += duration_intr;
            execution += std::to_string(current_time) + ", 0, scheduler called\n";
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

            // collect child-only trace block
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_seen = false;
            size_t parent_index = i;

            for (size_t j = i; j < trace_file.size(); j++) {
                auto [act, dur, pn] = parse_trace(trace_file[j]);
                if (skip && act == "IF_CHILD") { skip = false; continue; }
                else if (act == "IF_PARENT") {
                    skip = true;
                    parent_index = j;
                    if (exec_seen) break;
                } else if (skip && act == "ENDIF") { skip = false; continue; }
                else if (!skip && act == "EXEC") { exec_seen = true; child_trace.push_back(trace_file[j]); skip = true; }

                if (!skip) child_trace.push_back(trace_file[j]);
            }

            i = parent_index; // continue loop from parent's section

            // run child
            PCB new_process = { current.PID + 1, static_cast<int>(current.PID),
                                current.program_name, current.size, 0 };
            allocate_memory(&new_process);
            wait_queue.push_back(current);
            current = new_process;

            auto status_string = "time: " + std::to_string(current_time) + "; FORK";
            std::cout << status_string << "\n" << print_PCB(current, wait_queue) << "\n";
            system_status += status_string + "\n" + print_PCB(current, wait_queue) + "\n";

            auto [child_exec, child_status, child_time] =
                simulate_trace(child_trace, current_time, vectors, delays, external_files, current, wait_queue);
            execution += child_exec;
            system_status += child_status;
            current_time = child_time;
        }
        else if (activity == "EXEC") {
            auto [intr, t] = intr_boilerplate(current_time, 3, 10, vectors);
            execution += intr; current_time = t;

            unsigned int prog_size = get_size(program_name, external_files);
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) +
                         ", Program is " + std::to_string(prog_size) + "MB large\n";
            current_time += duration_intr;

            int prog_time = static_cast<int>(prog_size) * 15;
            execution += std::to_string(current_time) + ", " + std::to_string(prog_time) + ", loading program into memory\n";
            current_time += prog_time;

            execution += std::to_string(current_time) + ", 3, marking partition as occupied\n";
            current_time += 3;
            execution += std::to_string(current_time) + ", 6, updating PCB\n";
            current_time += 6;
            execution += std::to_string(current_time) + ", 0, scheduler called\n";
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

            // load the external program's own trace
            std::ifstream exec_trace_file("input_files/" + program_name + ".txt");
            std::vector<std::string> exec_traces;
            std::string line;
            while (std::getline(exec_trace_file, line)) exec_traces.push_back(line);

            current.program_name = program_name;
            current.size = prog_size;
            free_memory(&current);
            allocate_memory(&current);

            auto status_string = "time: " + std::to_string(current_time) + "; EXEC " + program_name;
            std::cout << status_string << "\n" << print_PCB(current, wait_queue) << "\n";
            system_status += status_string + "\n" + print_PCB(current, wait_queue) + "\n";

            auto [exec_execution, exec_status, exec_time] =
                simulate_trace(exec_traces, current_time, vectors, delays, external_files, current, wait_queue);
            execution += exec_execution;
            system_status += exec_status;
            current_time = exec_time;

            // return to caller after external program finishes (matches original flow)
            return {execution, system_status, current_time};
        }
        // else: ignore unknown activities silently
    }

    // wrap up
    free_memory(&current);
    if (!wait_queue.empty()) {
        current = wait_queue.front();
        wait_queue.erase(wait_queue.begin());
    }
    return {execution, system_status, current_time};
}
