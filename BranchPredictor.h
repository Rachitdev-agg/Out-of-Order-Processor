#pragma once
#include "Basics.h"
#include <iostream>
#include <vector>

class BranchPredictor {
public:
    int total_branches = 0;
    int correct_predictions = 0;

    int predict(int current_pc, int imm, OpCode op);
    void update(int pc, int actual_target, bool taken, bool was_correct);
};
