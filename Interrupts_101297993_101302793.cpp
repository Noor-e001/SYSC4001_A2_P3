#include "interrupts_101297993_101302793.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <cctype>

static TraceKind parseKind(const std::string& s) {
    std::string u = s;
    std::transform(u.begin(), u.end(), u.begin(), ::toupper);
    if (u == "CPU") return TraceKind::CPU;
    if (u == "SYSCALL") return TraceKind::SYSCALL;
    if (u == "END_IO") return TraceKind::END_IO;
    if (u == "FORK") return TraceKind::FORK;
    if (u == "EXEC") return TraceKind::EXEC;
    if (u == "IF_CHILD") return TraceKind::IF_CHILD;
    if (u == "IF_PARENT") return TraceKind::IF_PARENT;
    if (u == "ENDIF") return TraceKind::ENDIF;
    throw std::runtime_error("Unknown trace kind: " + s);
}

Simulator::Simulator(const std::string& inputDir, const std::string& outputDir) {
    partitions = {{1,40,"free"},{2,25,"free"},{3,15,"free"},{4,10,"free"},{5,8,"free"},{6,2,"init"}};
    pcbTable[0] = {0, "init", 6, 1, ProcState::RUNNING};
    execLog.open(outputDir + "/execution.txt");
    statusLog.open(outputDir + "/system_status.txt");
}

void Simulator::loadAll() { loadExternalFiles(); }

void Simulator::loadExternalFiles() {
    std::ifstream in("input_files/external_files.txt");
    if(!in) throw std::runtime_error("missing external_files.txt");
    std::string name; unsigned int sz;
    while(in >> name >> sz) externalMap[name] = sz;
}

void Simulator::loadTrace(const std::string& fileName, std::vector<TraceLine>& out) {
    std::ifstream in("input_files/" + fileName);
    std::string line;
    while(std::getline(in,line)) {
        if(line.empty()) continue;
        auto pos = line.find(',');
        std::string left = line.substr(0,pos);
        std::string right = line.substr(pos+1);
        auto trim = [](std::string &s){ while(!s.empty() && isspace(s.front())) s.erase(s.begin()); while(!s.empty() && isspace(s.back())) s.pop_back(); };
        trim(left); trim(right);
        std::stringstream ls(left); std::string type; ls >> type;
        TraceLine tl; tl.kind = parseKind(type); tl.raw = line;
        if(tl.kind == TraceKind::EXEC){ ls >> tl.argStr; tl.value = std::stoi(right); }
        else tl.value = std::stoi(right);
        out.push_back(tl);
    }
}

void Simulator::run() {
    std::vector<TraceLine> trace;
    loadTrace("trace.txt", trace);
    runProgram(trace);
}

void Simulator::runProgram(std::vector<TraceLine>& trace) {
    for(auto &t : trace) {
        switch(t.kind){
            case TraceKind::CPU: handleCPU(t.value); break;
            case TraceKind::SYSCALL: handleSYSCALL(t.value); break;
            case TraceKind::END_IO: handleENDIO(t.value); break;
            case TraceKind::FORK: handleFORK(t.value); break;
            case TraceKind::EXEC: handleEXEC(t.argStr, t.value); break;
            default: break;
        }
    }
}

void Simulator::handleCPU(int ms){ execLogLine(ms,"CPU Burst"); }
void Simulator::handleSYSCALL(int dev){ execLogLine(250,"SYSCALL ISR"); }
void Simulator::handleENDIO(int dev){ execLogLine(80,"END_IO serviced"); }

void Simulator::handleFORK(int isrMs){
    execLogLine(isrMs,"FORK invoked");
    PCB parent = pcbTable[currentPid];
    int childPid = nextPid++;
    PCB child = parent;
    child.pid = childPid;
    pcbTable[childPid] = child;
    snapshotSystem("FORK");
}

void Simulator::handleEXEC(const std::string& progName, int isrMs){
    execLogLine(isrMs,"EXEC "+progName);
    unsigned int size = externalMap[progName];
    int part = findFreePartition(size);
    markPartition(part, progName);
    snapshotSystem("EXEC " + progName);
}

void Simulator::execLogLine(int duration,const std::string& msg){
    execLog<<nowMs<<", "<<duration<<", "<<msg<<"\n"; nowMs+=duration;
}

int Simulator::findFreePartition(unsigned int sizeMB) const{
    for(int i=0;i<partitions.size();++i)
        if(partitions[i].code=="free" && partitions[i].sizeMB>=sizeMB)
            return i;
    return -1;
}

void Simulator::markPartition(int idx,const std::string& code){ partitions[idx].code=code; }

void Simulator::snapshotSystem(const std::string& trace){
    statusLog<<"time: "<<nowMs<<"; "<<trace<<"\n";
    for(auto &[pid,pcb]:pcbTable){
        statusLog<<pid<<" "<<pcb.programName<<" "<<pcb.partitionNumber<<" "<<pcb.programSizeMB<<" "<<(pcb.state==ProcState::RUNNING?"running":"waiting")<<"\n";
    }
    statusLog<<"\n";
}

void Simulator::callScheduler(){}
