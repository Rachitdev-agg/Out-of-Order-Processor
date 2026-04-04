#include <iostream>
#include "Processor.h"

using namespace std;
int main() {
    ProcessorConfig config;
    Processor cpu = Processor(config);
    cpu.loadProgram("programs/code4.txt");
    int c = 0;
    while(cpu.step()) {
        cout << "Cycle " << cpu.clock_cycle << ":" << endl;
        cout << "  Fetch: " << cpu.fetch_valid << " PC=" << cpu.pc << " count=" << cpu.rob_count << endl;
        for(int i=0; i<cpu.rob_count; i++) {
           int idx = (cpu.rob_head + i) % cpu.ROB.size();
           cout << "  ROB " << idx << " pc=" << cpu.ROB[idx].pc << " type=" << cpu.ROB[idx].type << " ready=" << cpu.ROB[idx].ready << endl; 
        }
        c++;
        if (c > 45) break; 
    }
    return 0;
}
