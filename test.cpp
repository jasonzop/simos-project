// Jason Parmar

#include "SimOS.h"
#include <iostream>

int main()
{
    SimOS sim(1, 256, 64);

    sim.NewProcess();              // PID 1
    sim.AccessMemoryAddress(0);    // page 0
    sim.AccessMemoryAddress(64);   // page 1

    std::cout << "Memory before exit: " << sim.GetMemory().size() << std::endl;

    sim.SimExit();

    std::cout << "CPU after exit: " << sim.GetCPU() << std::endl;
    std::cout << "Memory after exit: " << sim.GetMemory().size() << std::endl;

    return 0;
}