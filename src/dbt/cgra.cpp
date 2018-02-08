#include <simulator/cgraSimulator.h>
#include <isa/cgraIsa.h>

#include <iostream>


// ldi 10 | rt LEFT | NOP            | ldi 5
// NOP    | rt UP   | add LEFT RIGHT | addi 5 UP
// ldi 12 | rt LEFT | mul UP LEFT    | NOP

uint8_t dummy1[] = {
  0xBA, 0x00, 0x0F, 0x10, 0xB5, 0x00, 0x00, 0xF3, 0x11, 0x19,
  0xB2, 0x00, 0xBC, 0x00, 0x0F, 0x13, 0xB0, 0x00
};

int main(void)
{
  CgraSimulator sim;
  sim.configure(dummy1, sizeof(dummy1));

  for (int i = 0; i < 5; ++i)
  {
    std::cout << "================= Step " << i << " =================" << std::endl;
    sim.doStep();
    sim.print();
  }

  return 0;
}
