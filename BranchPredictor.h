#pragma once
#include <unordered_map>
#include "Basics.h"

class BranchPredictor {
public:
    int total_branches = 0;
    int correct_predictions = 0;

    std::unordered_map<int, int> counters;  // pc -> 2-bit saturating counter

    int predict(int current_pc, int imm, OpCode op);
    void update(int pc, int actual_target, bool taken, bool was_correct);
};
