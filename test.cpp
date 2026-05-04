// Jason Parmar

#include "SimOS.h"
#include <iostream>

int main()
{
    SimOS sim(1, 1024, 64);

    sim.NewProcess(); // PID 1
    sim.SimFork();    // PID 2 child of 1

    sim.TimerInterrupt(); // PID 2 runs
    sim.DiskReadRequest(0, "child.txt"); // PID 2 goes to disk

    sim.SimExit(); // PID 1 exits, should kill child PID 2 from disk

    std::cout << "CPU: " << sim.GetCPU() << std::endl;
    std::cout << "Disk current PID: " << sim.GetDisk(0).PID << std::endl;
    std::cout << "Disk queue size: " << sim.GetDiskQueue(0).size() << std::endl;

    return 0;
}