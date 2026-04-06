#pragma once
#include <string>
#include <vector>

enum class OpCode {
  ADD,
  SUB,
  ADDI,
  MUL,
  DIV,
  REM,
  LW,
  SW,
  BEQ,
  BNE,
  BLT,
  BLE,
  J,
  SLT,
  SLTI,
  AND,
  OR,
  XOR,
  ANDI,
  ORI,
  XORI
};
enum class UnitType { ADDER, MULTIPLIER, DIVIDER, LOADSTORE, BRANCH, LOGIC };

struct Instruction {
  OpCode op;
  int dest;
  int src1;
  int src2;
  int imm;
  int pc;
};

struct ProcessorConfig {
  int num_regs = 32;
  int rob_size = 64;
  int mem_size = 1024;

  int logic_lat = 1;
  int add_lat = 2;
  int mul_lat = 4;
  int div_lat = 5;
  int mem_lat = 4;

  int logic_rs_size = 4;
  int adder_rs_size = 4;
  int mult_rs_size = 2;
  int div_rs_size = 2;
  int br_rs_size = 2;
  int lsq_rs_size = 32;
};

struct ROBEntry {
  bool busy = false;
  bool ready = false;
  int dest = -1; // architectural register index (-1 if no writeback)
  int value = 0;
  bool exception = false;
  std::string type = "alu"; // "alu", "load", "store", "branch"
  int pc = -1;              // PC of this instruction
  int predicted_pc =
      -1; // predicted next-PC stored at decode time (for branches)
};

struct RSEntry {
  bool busy = false;
  OpCode op;
  int val1 = 0, val2 = 0;   // operand values when ready
  int tag1 = -1, tag2 = -1; // ROB tags we're waiting on
  bool ready1 = false, ready2 = false;
  int dest_tag = -1; // ROB slot this result writes to
  int imm = 0;
  int predicted_pc = -1; // for branches
  int inst_pc = -1;      // instruction PC, used to find oldest entry
  int cycles_left = 0;   // per-entry latency countdown (LSQ only)
};