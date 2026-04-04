#include "Processor.h"

Processor::Processor(ProcessorConfig &config) {
  pc = 0;
  clock_cycle = 0;
  ARF.resize(config.num_regs, 0);
  Memory.resize(config.mem_size);

  RAT.assign(config.num_regs, -1);
  ROB.resize(config.rob_size);
  lsq_max_size = config.lsq_rs_size;

  // Instantiate Hardware Units
  units.push_back(
      ExecutionUnit(UnitType::ADDER, config.add_lat, config.adder_rs_size));
  units.push_back(
      ExecutionUnit(UnitType::MULTIPLIER, config.mul_lat, config.mult_rs_size));
  units.push_back(
      ExecutionUnit(UnitType::DIVIDER, config.div_lat, config.div_rs_size));
  units.push_back(
      ExecutionUnit(UnitType::BRANCH, config.add_lat, config.br_rs_size));
  units.push_back(
      ExecutionUnit(UnitType::LOGIC, config.logic_lat, config.logic_rs_size));

  lsq = new LoadStoreQueue(config.mem_lat, config.lsq_rs_size);
}

void Processor::loadProgram(const std::string &filename) {
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
      if (line[i] == ',')
        line[i] = ' ';
    }

    // split into tokens
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string tok;
    while (ss >> tok)
      tokens.push_back(tok);

    if (tokens.empty())
      continue;

    // data directive: .A: 1 2 3 ...
    if (tokens[0][0] == '.') {
      std::string arrName =
          tokens[0].substr(1, tokens[0].size() - 2); // strip . and :
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
    inst.dest = 0;
    inst.src1 = 0;
    inst.src2 = 0;
    inst.imm = 0;
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
      inst.imm = std::stoi(tokens[3]);
    } else if (op == "andi") {
      inst.op = OpCode::ANDI;
      inst.dest = std::stoi(tokens[1].substr(1));
      inst.src1 = std::stoi(tokens[2].substr(1));
      inst.imm = std::stoi(tokens[3]);
    } else if (op == "ori") {
      inst.op = OpCode::ORI;
      inst.dest = std::stoi(tokens[1].substr(1));
      inst.src1 = std::stoi(tokens[2].substr(1));
      inst.imm = std::stoi(tokens[3]);
    } else if (op == "xori") {
      inst.op = OpCode::XORI;
      inst.dest = std::stoi(tokens[1].substr(1));
      inst.src1 = std::stoi(tokens[2].substr(1));
      inst.imm = std::stoi(tokens[3]);
    } else if (op == "slti") {
      inst.op = OpCode::SLTI;
      inst.dest = std::stoi(tokens[1].substr(1));
      inst.src1 = std::stoi(tokens[2].substr(1));
      inst.imm = std::stoi(tokens[3]);
    } else if (op == "lw") {
      inst.op = OpCode::LW;
      inst.dest = std::stoi(tokens[1].substr(1));
      int paren = tokens[2].find('(');
      std::string arrName = tokens[2].substr(0, paren);
      std::string regStr =
          tokens[2].substr(paren + 1, tokens[2].size() - paren - 2);
      inst.src1 = std::stoi(regStr.substr(1));
      inst.imm = arrayMap[arrName];
    } else if (op == "sw") {
      inst.op = OpCode::SW;
      inst.src2 = std::stoi(tokens[1].substr(1));
      int paren = tokens[2].find('(');
      std::string arrName = tokens[2].substr(0, paren);
      std::string regStr =
          tokens[2].substr(paren + 1, tokens[2].size() - paren - 2);
      inst.src1 = std::stoi(regStr.substr(1));
      inst.imm = arrayMap[arrName];
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

void Processor::flush() {
  fetch_valid = false;
  for (int i = 0; i < (int)RAT.size(); i++)
    RAT[i] = -1;
  rob_tail = rob_head;
  rob_count = 0;
  for (int i = 0; i < (int)units.size(); i++) {
    for (int j = 0; j < (int)units[i].rs.size(); j++) {
      units[i].rs[j].busy = false;
    }
    while (!units[i].pipeline.empty())
      units[i].pipeline.pop();
    units[i].has_result = false;
    units[i].has_exception = false;
  }
  while (!lsq->lsq.empty())
    lsq->lsq.pop();
  lsq->has_result = false;
  lsq->has_exception = false;
  lsq->store_ready = false;
  lsq->cycles_left = 0;
}

void Processor::broadcastOnCDB() {
  for (int i = 0; i < (int)units.size(); i++) {
    if (units[i].has_result) {
      int tag = units[i].result_tag;
      int val = units[i].result_val;
      for (auto &u : units)
        u.capture(tag, val);
      lsq->capture(tag, val);

      if (tag != -1) {
        ROB[tag].value = val;
        ROB[tag].ready = true;
        if (units[i].has_exception) {
          ROB[tag].exception = true;
        }
      }
    }
  }
  if (lsq->has_result) {
    int tag = lsq->result_tag;
    int val = lsq->result_val;
    for (auto &u : units)
      u.capture(tag, val);
    lsq->capture(tag, val);
    if (tag != -1) {
      ROB[tag].value = val;
      ROB[tag].ready = true;
      if (lsq->has_exception) {
        ROB[tag].exception = true;
      }
    }
  }
}

void Processor::stageFetch() {
  if (fetch_valid)
    return;
  if (pc >= (int)inst_memory.size())
    return;

  fetch_inst = inst_memory[pc];
  fetch_pred_pc = bp.predict(pc, fetch_inst.imm, fetch_inst.op);
  fetch_valid = true;

  if (fetch_inst.op == OpCode::J) {
    pc = fetch_inst.imm;
  } else if (fetch_inst.op == OpCode::BEQ || fetch_inst.op == OpCode::BNE ||
             fetch_inst.op == OpCode::BLT || fetch_inst.op == OpCode::BLE) {
    pc = fetch_pred_pc;
  } else {
    pc = pc + 1;
  }
}

void Processor::stageDecode() {
  if (!fetch_valid)
    return;

  Instruction inst = fetch_inst;
  if (rob_count == (int)ROB.size())
    return;

  UnitType type = UnitType::ADDER;
  if (inst.op == OpCode::ADD || inst.op == OpCode::SUB ||
      inst.op == OpCode::ADDI || inst.op == OpCode::SLT ||
      inst.op == OpCode::SLTI)
    type = UnitType::ADDER;
  else if (inst.op == OpCode::MUL)
    type = UnitType::MULTIPLIER;
  else if (inst.op == OpCode::DIV || inst.op == OpCode::REM)
    type = UnitType::DIVIDER;
  else if (inst.op == OpCode::LW || inst.op == OpCode::SW)
    type = UnitType::LOADSTORE;
  else if (inst.op == OpCode::BEQ || inst.op == OpCode::BNE ||
           inst.op == OpCode::BLT || inst.op == OpCode::BLE)
    type = UnitType::BRANCH;
  else if (inst.op == OpCode::J) {
  } else {
    type = UnitType::LOGIC;
  }

  int rs_idx = -1;
  ExecutionUnit *target_unit = nullptr;
  if (inst.op != OpCode::J) {
    if (type == UnitType::LOADSTORE) {
      if ((int)lsq->lsq.size() >= lsq_max_size)
        return;
    } else {
      for (auto &u : units) {
        if (u.name == type) {
          target_unit = &u;
          break;
        }
      }
      if (target_unit) {
        for (int i = 0; i < (int)target_unit->rs.size(); i++) {
          if (!target_unit->rs[i].busy) {
            rs_idx = i;
            break;
          }
        }
        if (rs_idx == -1)
          return;
      }
    }
  }

  int rob_idx = rob_tail;
  rob_tail = (rob_tail + 1) % ROB.size();
  rob_count++;

  ROB[rob_idx].busy = true;
  ROB[rob_idx].ready = false;
  ROB[rob_idx].exception = false;
  ROB[rob_idx].pc = inst.pc;
  ROB[rob_idx].value = 0;
  ROB[rob_idx].predicted_pc = fetch_pred_pc; // store prediction at decode time

  if (inst.op == OpCode::SW || inst.op == OpCode::BEQ ||
      inst.op == OpCode::BNE || inst.op == OpCode::BLT ||
      inst.op == OpCode::BLE || inst.op == OpCode::J) {
    ROB[rob_idx].dest = -1;
    if (inst.op == OpCode::SW)
      ROB[rob_idx].type = "store";
    else
      ROB[rob_idx].type = "branch";
  } else {
    if (inst.dest == 0) {
      ROB[rob_idx].dest = -1;
    } else {
      ROB[rob_idx].dest = inst.dest;
      // Removed RAT[inst.dest] = rob_idx; from here, moved to end.
    }
    if (inst.op == OpCode::LW)
      ROB[rob_idx].type = "load";
    else
      ROB[rob_idx].type = "alu";
  }

  if (inst.op == OpCode::J) {
    ROB[rob_idx].ready = true;
  } else {
    RSEntry rse;
    rse.busy = true;
    rse.op = inst.op;
    rse.dest_tag = rob_idx;
    rse.imm = inst.imm;
    rse.inst_pc = inst.pc;
    if (type == UnitType::BRANCH)
      rse.predicted_pc = fetch_pred_pc;

    bool read_src1 = true, read_src2 = false;

    if (inst.op == OpCode::ADD || inst.op == OpCode::SUB ||
        inst.op == OpCode::MUL || inst.op == OpCode::DIV ||
        inst.op == OpCode::REM || inst.op == OpCode::AND ||
        inst.op == OpCode::OR || inst.op == OpCode::XOR ||
        inst.op == OpCode::SLT || inst.op == OpCode::SW ||
        inst.op == OpCode::BEQ || inst.op == OpCode::BNE ||
        inst.op == OpCode::BLT || inst.op == OpCode::BLE) {
      read_src2 = true;
    }

    auto read_reg = [&](int reg, int &val, int &tag, bool &ready) {
      if (reg == 0) {
        val = 0;
        tag = -1;
        ready = true;
      } else {
        if (RAT[reg] != -1) {
          int p_tag = RAT[reg];
          if (ROB[p_tag].ready) {
            val = ROB[p_tag].value;
            tag = -1;
            ready = true;
          } else {
            tag = p_tag;
            ready = false;
          }
        } else {
          val = ARF[reg];
          tag = -1;
          ready = true;
        }
      }
    };

    if (read_src1)
      read_reg(inst.src1, rse.val1, rse.tag1, rse.ready1);
    else
      rse.ready1 = true;

    if (read_src2)
      read_reg(inst.src2, rse.val2, rse.tag2, rse.ready2);
    else
      rse.ready2 = true;

    if (type == UnitType::LOADSTORE) {
      lsq->lsq.push(rse);
    } else {
      target_unit->rs[rs_idx] = rse;
    }
  }

  // Update RAT AFTER reading sources!
  if (inst.op != OpCode::SW && inst.op != OpCode::BEQ &&
      inst.op != OpCode::BNE && inst.op != OpCode::BLT &&
      inst.op != OpCode::BLE && inst.op != OpCode::J) {
    if (inst.dest != 0) {
      RAT[inst.dest] = rob_idx;
    }
  }

  fetch_valid = false;
}

void Processor::stageExecuteAndBroadcast() {
  for (auto &u : units) {
    u.executeCycle();
  }
  lsq->executeCycle(Memory);

  if (lsq->store_ready) {
    int tag = lsq->store_tag;
    if (tag != -1) {
      ROB[tag].dest = lsq->store_addr;  // Store memory addr here
      ROB[tag].value = lsq->store_data; // Store memory value here
      ROB[tag].ready = true;
    }
    lsq->store_ready = false;
  }
}

void Processor::stageCommit() {
  if (rob_count == 0)
    return;
  int head = rob_head;
  ROBEntry &entry = ROB[head];

  if (!entry.ready) {
    return;
  }

  if (entry.exception) {
    pc = entry.pc;
    exception = true;
    flush();
    return;
  }

  if (entry.type == "store") {
    if (entry.dest < 0 || entry.dest >= (int)Memory.size()) {
      pc = entry.pc;
      exception = true;
      flush();
      return;
    } else {
      Memory[entry.dest] = entry.value;
    }
  }

  if (entry.type == "branch") {
    Instruction inst = inst_memory[entry.pc];
    if (inst.op != OpCode::J) {
      int predicted_target =
          entry.predicted_pc; // read from ROB (stored at decode)
      int actual_target = entry.value;
      bool taken = (actual_target != entry.pc + 1);
      bool correct = (predicted_target == actual_target);

      bp.update(entry.pc, actual_target, taken, correct);

      if (!correct) {
        pc = actual_target;
        // Retire this branch entry before flushing the rest
        ROB[head].busy = false;
        rob_head = (rob_head + 1) % ROB.size();
        rob_count--;
        flush();
        return;
      }
    }
  }

  if (entry.type == "load" || entry.type == "alu") {
    if (entry.dest != -1) {
      ARF[entry.dest] = entry.value;
      if (RAT[entry.dest] == head) {
        RAT[entry.dest] = -1;
      }
    }
  }

  ROB[head].busy = false;
  rob_head = (rob_head + 1) % ROB.size();
  rob_count--;
}

bool Processor::step() {
  if (exception)
    return false;
  if (pc >= (int)inst_memory.size() && !fetch_valid && rob_count == 0)
    return false;

  clock_cycle++;
  stageCommit();
  if (exception)
    return true;
  stageExecuteAndBroadcast();
  stageDecode();
  stageFetch();
  broadcastOnCDB();

  return true;
}

void Processor::dumpArchitecturalState() {
  std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
  for (int i = 0; i < (int)ARF.size(); i++) {
    std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
    if ((i + 1) % 8 == 0)
      std::cout << std::endl;
  }
  if (exception) {
    std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
  }
  std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/"
            << bp.total_branches << " correct.\n";
}
