#include "BranchPredictor.h"

int BranchPredictor::predict(int current_pc, int imm, OpCode op) {
    // unconditional jump doesn't need a predictor
    if (op == OpCode::J) return imm;

    // if no counter exists for this pc yet, start at 1 (weakly not taken)
    if (counters.find(current_pc) == counters.end())
        counters[current_pc] = 1;

    int counter = counters[current_pc];

    // >= 2 means predict taken, < 2 means predict not taken
    if (counter >= 2)
        return imm;         // predict taken -> jump to label
    else
        return current_pc + 1;  // predict not taken -> next instruction
}

void BranchPredictor::update(int pc, int actual_target, bool taken, bool was_correct) {
    total_branches++;
    if (was_correct) correct_predictions++;

    if (counters.find(pc) == counters.end())
        counters[pc] = 1;

    if (taken) {
        if (counters[pc] < 3) counters[pc]++;
    } else {
        if (counters[pc] > 0) counters[pc]--;
    }
}
