#include "ExecutionUnit.h"

ExecutionUnit::ExecutionUnit(UnitType type, int lat, int rs_size) {
    name = type;
    latency = lat;
    rs.resize(rs_size);
}

void ExecutionUnit::capture(int tag, int val) {
    for (int i = 0; i < (int)rs.size(); i++) {
        if (!rs[i].busy) continue;
        if (!rs[i].ready1 && rs[i].tag1 == tag) {
            rs[i].val1 = val;
            rs[i].ready1 = true;
        }
        if (!rs[i].ready2 && rs[i].tag2 == tag) {
            rs[i].val2 = val;
            rs[i].ready2 = true;
        }
    }
}

void ExecutionUnit::executeCycle() {
    has_result = false;
    has_exception = false;

    // pick oldest ready RS entry and start executing it
    int oldest_idx = -1;
    int oldest_pc  = -1;
    for (int i = 0; i < (int)rs.size(); i++) {
        if (rs[i].busy && rs[i].ready1 && rs[i].ready2) {
            if (oldest_idx == -1 || rs[i].inst_pc < oldest_pc) {
                oldest_idx = i;
                oldest_pc  = rs[i].inst_pc;
            }
        }
    }
    if (oldest_idx != -1) {
        pipeline.push({rs[oldest_idx], latency});
        rs[oldest_idx].busy = false;
    }

    // tick down all in-flight instructions
    int p_size = pipeline.size();
    for (int i = 0; i < p_size; i++) {
        auto front = pipeline.front();
        pipeline.pop();
        front.second--;

        if (front.second == 0) {
            RSEntry& entry = front.first;
            int val1 = entry.val1;
            int val2 = entry.val2;
            int imm  = entry.imm;
            int res  = 0;

            switch (entry.op) {
                case OpCode::ADD:  res = val1 + val2; break;
                case OpCode::SUB:  res = val1 - val2; break;
                case OpCode::ADDI: res = val1 + imm;  break;
                case OpCode::AND:  res = val1 & val2; break;
                case OpCode::OR:   res = val1 | val2; break;
                case OpCode::XOR:  res = val1 ^ val2; break;
                case OpCode::ANDI: res = val1 & imm;  break;
                case OpCode::ORI:  res = val1 | imm;  break;
                case OpCode::XORI: res = val1 ^ imm;  break;
                case OpCode::SLT:  res = (val1 < val2) ? 1 : 0; break;
                case OpCode::SLTI: res = (val1 < imm)  ? 1 : 0; break;
                case OpCode::MUL:  res = val1 * val2; break;
                case OpCode::DIV:
                    if (val2 == 0) { has_exception = true; res = 0; }
                    else res = val1 / val2;
                    break;
                case OpCode::REM:
                    if (val2 == 0) { has_exception = true; res = 0; }
                    else res = val1 % val2;
                    break;
                case OpCode::BEQ: res = (val1 == val2) ? entry.imm : entry.inst_pc + 1; break;
                case OpCode::BNE: res = (val1 != val2) ? entry.imm : entry.inst_pc + 1; break;
                case OpCode::BLT: res = (val1 <  val2) ? entry.imm : entry.inst_pc + 1; break;
                case OpCode::BLE: res = (val1 <= val2) ? entry.imm : entry.inst_pc + 1; break;
                case OpCode::J:   res = entry.imm; break;
                default: break;
            }

            result_val = res;
            result_tag = entry.dest_tag;
            has_result = true;
        } else {
            pipeline.push(front);
        }
    }
}
