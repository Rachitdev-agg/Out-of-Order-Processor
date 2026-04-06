#pragma once
#include "Basics.h"
#include <iostream>
#include <queue>
#include <string>
#include <vector>

struct LSQResult {
  bool is_load;       // true = load (broadcast on CDB), false = store
  int result_val;     // load: value read from memory
  int result_tag;     // load: ROB tag to write back
  bool has_exception; // load: out-of-bounds access
  int store_addr;     // store: target memory address
  int store_data;     // store: value to write
  int store_tag;      // store: ROB tag to mark ready
};

class LoadStoreQueue {
public:
  int latency;

  std::queue<RSEntry> lsq; // in-order queue of load/store entries

  // Legacy single-result fields kept for flush() compatibility in Processor.cpp
  bool has_result = false;
  bool has_exception = false;
  bool store_ready = false;
  int result_val = 0;
  int result_tag = -1;
  int store_addr = 0;
  int store_data = 0;
  int store_tag = -1;
  int cycles_left = 0; // kept only so Processor::flush() compiles

  // All results produced this cycle (loads + stores)
  std::vector<LSQResult> results_this_cycle;

  LoadStoreQueue(int lat, int size);

  void capture(int tag, int val);

  // Processes ALL entries in parallel with per-entry latency.
  // Completed entries are removed; results collected in results_this_cycle.
  void executeCycle(std::vector<int> &Memory);
};