// Jason Parmar

#include "SimOS.h"
#include <iostream>

int main()
{
    SimOS sim(1, 1024, 64);

    sim.NewProcess(); // PID 1
    sim.SimFork();    // PID 2 child of 1

    sim.TimerInterrupt(); // PID 2 runs
    sim.SimFork();        // PID 3 child of 2

    sim.TimerInterrupt(); // PID 1 runs
    sim.SimExit();        // PID 1 exits, should kill PID 2 and PID 3 too

    std::cout << "CPU: " << sim.GetCPU() << std::endl;
    std::cout << "Ready Queue size: " << sim.GetReadyQueue().size() << std::endl;
    std::cout << "Memory size: " << sim.GetMemory().size() << std::endl;

    return 0;
}