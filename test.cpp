// Jason Parmar

#include "SimOS.h"
#include <iostream>

int main()
{
    SimOS sim(1, 1024, 64);

    sim.NewProcess(); // PID 1
    sim.NewProcess(); // PID 2
    sim.NewProcess(); // PID 3

    sim.DiskReadRequest(0, "one.txt"); // PID 1 on disk
    sim.DiskReadRequest(0, "two.txt"); // PID 2 in disk queue

    sim.SimExit(); // PID 3 exits

    std::cout << "CPU: " << sim.GetCPU() << std::endl;
    std::cout << "Disk current PID: " << sim.GetDisk(0).PID << std::endl;
    std::cout << "Disk queue size: " << sim.GetDiskQueue(0).size() << std::endl;

    return 0;
}