#include "Processor.h"
#include <iostream>
int main() {
  ProcessorConfig config;
  Processor cpu = Processor(config);
  cpu.loadProgram("programs/code5.txt");
  int c = 0;
  while (cpu.step()) {
    c++;
    if (c > 35)
      break;
  }
  return 0;
}
