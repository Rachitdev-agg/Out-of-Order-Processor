#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip>
#include "Basics.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"

class Processor {
public:
    int pc;
    int clock_cycle;

    // pipeline registers
    bool fetch_valid = false;
    Instruction fetch_inst;
    int fetch_pred_pc = -1;

    std::vector<Instruction> inst_memory;

    // architectural state (do not change)
    std::vector<int> ARF; // regFile
    std::vector<int> Memory; // Memory
    bool exception = false; // exception bit

    // register alias table / reorder buffer
    std::vector<int> RAT;
    std::vector<ROBEntry> ROB;
    int rob_head = 0;
    int rob_tail = 0;
    int rob_count = 0;
    int lsq_max_size = 0;

    std::vector<ExecutionUnit> units;
    LoadStoreQueue* lsq;
    BranchPredictor bp;

    Processor(ProcessorConfig& config);

    void loadProgram(const std::string& filename);

    void flush();
    void broadcastOnCDB();
    void stageFetch();
    void stageDecode();
    void stageExecuteAndBroadcast();
    void stageCommit();
    bool step();
    void dumpArchitecturalState();
};
