#include "BranchPredictor.h"

int BranchPredictor::predict(int current_pc, int imm, OpCode op) {
    return current_pc + imm;
}

void BranchPredictor::update(int pc, int actual_target, bool taken, bool was_correct) {
    total_branches++;
    if (was_correct) {
        correct_predictions++;
    }
}
