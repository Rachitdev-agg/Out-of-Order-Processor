#include "LoadStoreQueue.h"

LoadStoreQueue::LoadStoreQueue(int lat, int size) {
  latency = lat;
  cycles_left = lat;
}

void LoadStoreQueue::capture(int tag, int val) {
  // Snoop all entries — std::queue doesn't support iteration, so rebuild it.
  std::queue<RSEntry> temp;
  while (!lsq.empty()) {
    RSEntry entry = lsq.front();
    lsq.pop();
    if (!entry.ready1 && entry.tag1 == tag) {
      entry.val1 = val;
      entry.ready1 = true;
    }
    if (!entry.ready2 && entry.tag2 == tag) {
      entry.val2 = val;
      entry.ready2 = true;
    }
    temp.push(entry);
  }
  lsq = temp;
}

void LoadStoreQueue::executeCycle(std::vector<int> &Memory) {
  // Reset legacy single-result fields each cycle.
  has_result = false;
  has_exception = false;
  store_ready = false;
  results_this_cycle.clear();

  if (lsq.empty())
    return;

  // Iterate every entry in the queue.  For each:
  //   - If operands are not ready: keep as-is (no latency decrement).
  //   - If operands ready: decrement per-entry cycles_left.
  //     When it hits 0: complete the operation and do NOT requeue the entry.
  std::queue<RSEntry> remaining;

  while (!lsq.empty()) {
    RSEntry entry = lsq.front();
    lsq.pop();

    if (!entry.ready1 || !entry.ready2) {
      // Operands not yet available — stall this entry, keep it unchanged.
      remaining.push(entry);
      continue;
    }

    // Operands ready: tick down.
    entry.cycles_left--;

    if (entry.cycles_left > 0) {
      // Still in-flight; keep it.
      remaining.push(entry);
      continue;
    }

    // cycles_left == 0: operation complete this cycle.
    int addr = entry.val1 + entry.imm;
    LSQResult res{};
    res.result_tag = entry.dest_tag;

    if (entry.op == OpCode::LW) {
      res.is_load = true;
      if (addr < 0 || addr >= (int)Memory.size()) {
        res.has_exception = true;
        res.result_val = 0;
      } else {
        res.has_exception = false;
        res.result_val = Memory[addr];
      }
      results_this_cycle.push_back(res);

      // Also populate legacy fields (last completing load wins if multiple,
      // but the full list in results_this_cycle is what the processor uses).
      has_result = true;
      has_exception = res.has_exception;
      result_val = res.result_val;
      result_tag = res.result_tag;

    } else { // SW
      res.is_load = false;
      res.store_addr = addr;
      res.store_data = entry.val2;
      res.store_tag = entry.dest_tag;
      results_this_cycle.push_back(res);

      // Legacy fields
      store_ready = true;
      store_addr = res.store_addr;
      store_data = res.store_data;
      store_tag = res.store_tag;
    }
    // Do NOT push back — entry is retired from the LSQ.
  }

  lsq = remaining;
}