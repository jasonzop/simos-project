// Jason Parmar

#include "SimOS.h"
#include <iostream>
#include <stdexcept>

int main()
{
    SimOS sim(1, 1024, 64);

    try
    {
        sim.DiskReadRequest(0, "test.txt");
    }
    catch (const std::logic_error&)
    {
        std::cout << "logic_error passed" << std::endl;
    }

    sim.NewProcess();

    try
    {
        sim.GetDisk(5);
    }
    catch (const std::out_of_range&)
    {
        std::cout << "out_of_range passed" << std::endl;
    }

    return 0;
}