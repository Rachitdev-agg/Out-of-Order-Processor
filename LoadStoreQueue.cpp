#include "LoadStoreQueue.h"

LoadStoreQueue::LoadStoreQueue(int lat, int size) {
    latency = lat;
    cycles_left = 0;
}

void LoadStoreQueue::capture(int tag, int val) {
    // need to snoop all entries in the queue
    // std::queue doesn't support iteration, so we rebuild it
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

void LoadStoreQueue::executeCycle(std::vector<int>& Memory) {
    has_result = false;
    has_exception = false;
    store_ready = false;

    if (lsq.empty()) return;

    RSEntry& front = lsq.front();

    // wait until both operands are ready before starting
    if (!front.ready1 || !front.ready2) return;

    if (cycles_left == 0)
        cycles_left = latency;

    cycles_left--;

    if (cycles_left == 0) {
        int addr = front.val1 + front.imm;

        if (front.op == OpCode::LW) {
            result_val = Memory[addr];
            result_tag = front.dest_tag;
            has_result = true;
        } else {  // SW
            store_addr = addr;
            store_data = front.val2;
            store_ready = true;
        }

        lsq.pop();
    }
}
