#include "Processor.h"

Processor::Processor(ProcessorConfig& config) {
    pc = 0;
    clock_cycle = 0;
    ARF.resize(config.num_regs, 0);
    Memory.resize(config.mem_size);

    // Instantiate Hardware Units
    // Adder
    // Multiplier
    // Divider
    // Branch Computation
    // Bitwise Logic
    // Load-Store Unit
}

void Processor::loadProgram(const std::string& filename) {
    std::ifstream file(filename);

    std::unordered_map<std::string, int> labelMap;
    std::unordered_map<std::string, int> arrayMap;
    std::vector<std::string> branchLabels;
    int memOffset = 0;

    std::string line;
    while (std::getline(file, line)) {

        // cut off inline comment
        int hashPos = line.find('#');
        if (hashPos != (int)std::string::npos)
            line = line.substr(0, hashPos);

        // replace commas with spaces so tokenizing works either way
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == ',') line[i] = ' ';
        }

        // split into tokens
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string tok;
        while (ss >> tok) tokens.push_back(tok);

        if (tokens.empty()) continue;

        // data directive: .A: 1 2 3 ...
        if (tokens[0][0] == '.') {
            std::string arrName = tokens[0].substr(1, tokens[0].size() - 2); // strip . and :
            arrayMap[arrName] = memOffset;
            for (int i = 1; i < (int)tokens.size(); i++)
                Memory[memOffset++] = std::stoi(tokens[i]);
            continue;
        }

        // label: ends with ':'
        if (tokens[0].back() == ':') {
            std::string lname = tokens[0].substr(0, tokens[0].size() - 1);
            labelMap[lname] = (int)inst_memory.size();
            continue;
        }

        // instruction
        Instruction inst;
        inst.dest = 0; inst.src1 = 0; inst.src2 = 0; inst.imm = 0;
        inst.pc = (int)inst_memory.size();
        std::string brLabel = "";
        std::string op = tokens[0];

        if (op == "add") {
            inst.op = OpCode::ADD;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "sub") {
            inst.op = OpCode::SUB;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "mul") {
            inst.op = OpCode::MUL;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "div") {
            inst.op = OpCode::DIV;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "rem") {
            inst.op = OpCode::REM;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "and") {
            inst.op = OpCode::AND;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "or") {
            inst.op = OpCode::OR;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "xor") {
            inst.op = OpCode::XOR;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "slt") {
            inst.op = OpCode::SLT;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.src2 = std::stoi(tokens[3].substr(1));
        } else if (op == "addi") {
            inst.op = OpCode::ADDI;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.imm  = std::stoi(tokens[3]);
        } else if (op == "andi") {
            inst.op = OpCode::ANDI;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.imm  = std::stoi(tokens[3]);
        } else if (op == "ori") {
            inst.op = OpCode::ORI;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.imm  = std::stoi(tokens[3]);
        } else if (op == "xori") {
            inst.op = OpCode::XORI;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.imm  = std::stoi(tokens[3]);
        } else if (op == "slti") {
            inst.op = OpCode::SLTI;
            inst.dest = std::stoi(tokens[1].substr(1));
            inst.src1 = std::stoi(tokens[2].substr(1));
            inst.imm  = std::stoi(tokens[3]);
        } else if (op == "lw") {
            inst.op = OpCode::LW;
            inst.dest = std::stoi(tokens[1].substr(1));
            int paren = tokens[2].find('(');
            std::string arrName = tokens[2].substr(0, paren);
            std::string regStr  = tokens[2].substr(paren + 1, tokens[2].size() - paren - 2);
            inst.src1 = std::stoi(regStr.substr(1));
            inst.imm  = arrayMap[arrName];
        } else if (op == "sw") {
            inst.op = OpCode::SW;
            inst.src2 = std::stoi(tokens[1].substr(1));
            int paren = tokens[2].find('(');
            std::string arrName = tokens[2].substr(0, paren);
            std::string regStr  = tokens[2].substr(paren + 1, tokens[2].size() - paren - 2);
            inst.src1 = std::stoi(regStr.substr(1));
            inst.imm  = arrayMap[arrName];
        } else if (op == "beq") {
            inst.op = OpCode::BEQ;
            inst.src1 = std::stoi(tokens[1].substr(1));
            inst.src2 = std::stoi(tokens[2].substr(1));
            brLabel = tokens[3];
        } else if (op == "bne") {
            inst.op = OpCode::BNE;
            inst.src1 = std::stoi(tokens[1].substr(1));
            inst.src2 = std::stoi(tokens[2].substr(1));
            brLabel = tokens[3];
        } else if (op == "blt") {
            inst.op = OpCode::BLT;
            inst.src1 = std::stoi(tokens[1].substr(1));
            inst.src2 = std::stoi(tokens[2].substr(1));
            brLabel = tokens[3];
        } else if (op == "ble") {
            inst.op = OpCode::BLE;
            inst.src1 = std::stoi(tokens[1].substr(1));
            inst.src2 = std::stoi(tokens[2].substr(1));
            brLabel = tokens[3];
        } else if (op == "j") {
            inst.op = OpCode::J;
            brLabel = tokens[1];
        }

        inst_memory.push_back(inst);
        branchLabels.push_back(brLabel);
    }

    // pass 2: fill in branch/jump target PCs
    for (int i = 0; i < (int)inst_memory.size(); i++) {
        if (!branchLabels[i].empty())
            inst_memory[i].imm = labelMap[branchLabels[i]];
    }
}

void Processor::flush() {}

void Processor::broadcastOnCDB() {}

void Processor::stageFetch() {}

void Processor::stageDecode() {}

void Processor::stageExecuteAndBroadcast() {}

void Processor::stageCommit() {}

bool Processor::step() {
    clock_cycle++;
    return true;
}

void Processor::dumpArchitecturalState() {
    std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
    for (int i = 0; i < (int)ARF.size(); i++) {
        std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
        if ((i+1) % 8 == 0) std::cout << std::endl;
    }
    if (exception) {
        std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
    }
    std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/" << bp.total_branches << " correct.\n";
}
