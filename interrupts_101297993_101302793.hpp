#ifndef INTERRUPTS_STUDENTS_HPP
#define INTERRUPTS_STUDENTS_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <fstream>

struct Partition {
    unsigned int number;
    unsigned int sizeMB;
    std::string code;
};

enum class ProcState { RUNNING, WAITING };

struct PCB {
    int pid;
    std::string programName;
    int partitionNumber;
    unsigned int programSizeMB;
    ProcState state;
};

struct ExternalFile {
    std::string name;
    unsigned int sizeMB;
};

enum class TraceKind {
    CPU, SYSCALL, END_IO, FORK, EXEC,
    IF_CHILD, IF_PARENT, ENDIF
};

struct TraceLine {
    TraceKind kind;
    std::string argStr;
    int value;
    std::string raw;
};

class Simulator {
public:
    explicit Simulator(const std::string& inputDir, const std::string& outputDir);
    void loadAll();
    void run();

private:
    void loadExternalFiles();
    void loadTrace(const std::string& fileName, std::vector<TraceLine>& out);
    void runProgram(std::vector<TraceLine>& trace);

    void handleCPU(int ms);
    void handleSYSCALL(int dev);
    void handleENDIO(int dev);
    void handleFORK(int isrMs);
    void handleEXEC(const std::string& progName, int isrMs);
    void callScheduler();
    int findFreePartition(unsigned int sizeMB) const;
    void markPartition(int partIdx, const std::string& code);
    void snapshotSystem(const std::string& currentTraceRaw);
    void execLogLine(int duration, const std::string& message);

    std::vector<Partition> partitions;
    std::unordered_map<std::string, unsigned int> externalMap;
    std::unordered_map<int, PCB> pcbTable;

    long long nowMs = 0;
    std::ofstream execLog;
    std::ofstream statusLog;
    int nextPid = 1;
    int currentPid = 0;
};

#endif
