#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "Basics.h"

class ExecutionUnit {
public:
    UnitType name;
    int latency;

    std::vector<RSEntry> rs;
    std::queue<std::pair<RSEntry, int>> pipeline;  // {entry, cycles_remaining}

    bool has_result = false;
    bool has_exception = false;
    int result_val = 0;
    int result_tag = -1;

    ExecutionUnit(UnitType type, int lat, int rs_size);

    void capture(int tag, int val);
    void executeCycle();
};
