#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "Basics.h"

class LoadStoreQueue {
public:
    int latency;
    int cycles_left = 0;

    std::queue<RSEntry> lsq;  // in-order queue of load/store entries

    bool has_result = false;   // true when a load finishes — broadcast on CDB
    bool has_exception = false;
    bool store_ready = false;  // true when a store has computed address+value, ready to commit
    int result_val = 0;
    int result_tag = -1;

    // exposed to processor at commit time for stores
    int store_addr = 0;
    int store_data = 0;
    int store_tag = -1;

    LoadStoreQueue(int lat, int size);

    void capture(int tag, int val);
    void executeCycle(std::vector<int>& Memory);
};
