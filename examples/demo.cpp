#include "SimOS.h"

#include <iostream>

int main()
{
    SimOS os(2, 1024, 64);

    os.NewProcess(); // PID 1 starts on the CPU.
    os.SimFork();    // PID 1 creates child PID 2.
    os.AccessMemoryAddress(0x0040);
    os.AccessMemoryAddress(0x0080);

    os.TimerInterrupt();                  // Round-robin switches to PID 2.
    os.DiskReadRequest(0, "config.json"); // PID 2 blocks on disk 0.

    std::cout << "Running process: PID " << os.GetCPU() << '\n';
    std::cout << "Disk 0 request: " << os.GetDisk(0).fileName << '\n';
    std::cout << "Resident pages: " << os.GetMemory().size() << '\n';

    return 0;
}
