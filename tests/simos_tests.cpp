#include "SimOS.h"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
void Check(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

template <typename Exception, typename Callable>
void CheckThrows(Callable&& callable, const std::string& message)
{
    try
    {
        std::forward<Callable>(callable)();
    }
    catch (const Exception&)
    {
        return;
    }
    catch (...)
    {
        throw std::runtime_error(message + " (wrong exception type)");
    }

    throw std::runtime_error(message + " (no exception thrown)");
}

void TestConstructorValidation()
{
    CheckThrows<std::invalid_argument>([] { SimOS os(-1, 1024, 64); },
                                       "Negative disk counts must be rejected");
    CheckThrows<std::invalid_argument>([] { SimOS os(1, 1024, 0); },
                                       "A zero-byte page must be rejected");
}

void TestRoundRobinScheduling()
{
    SimOS os(1, 1024, 64);
    os.NewProcess();
    os.NewProcess();
    os.NewProcess();

    Check(os.GetCPU() == 1, "The first process should immediately own the CPU");
    Check(os.GetReadyQueue() == std::deque<int>({2, 3}),
          "Later processes should enter the ready queue in creation order");

    os.TimerInterrupt();
    Check(os.GetCPU() == 2, "A timer interrupt should dispatch the next process");
    Check(os.GetReadyQueue() == std::deque<int>({3, 1}),
          "The interrupted process should move to the back of the queue");
}

void TestDiskQueueingAndCompletion()
{
    SimOS os(1, 1024, 64);
    os.NewProcess();
    os.NewProcess();

    os.DiskReadRequest(0, "alpha.txt");
    Check(os.GetCPU() == 2, "A process should block while its disk request is active");
    Check(os.GetDisk(0).PID == 1, "The first request should start immediately");

    os.DiskReadRequest(0, "beta.txt");
    Check(os.GetCPU() == NO_PROCESS, "The CPU should be idle when every process is blocked");
    Check(os.GetDiskQueue(0).size() == 1, "A busy disk should queue later requests");

    os.DiskJobCompleted(0);
    Check(os.GetCPU() == 1, "A completed request should make its process runnable");
    Check(os.GetDisk(0).PID == 2, "The next queued disk request should start automatically");
}

void TestLeastRecentlyUsedReplacement()
{
    SimOS os(0, 128, 64); // Two physical frames.
    os.NewProcess();

    os.AccessMemoryAddress(0);
    os.AccessMemoryAddress(64);
    os.AccessMemoryAddress(0);   // Page 0 becomes most recently used.
    os.AccessMemoryAddress(128); // Page 1 should be evicted.

    const MemoryUsage memory = os.GetMemory();
    Check(memory.size() == 2, "Physical memory should remain bounded by frame count");

    bool hasPageZero = false;
    bool hasPageTwo = false;
    for (const MemoryItem& item : memory)
    {
        hasPageZero = hasPageZero || item.pageNumber == 0;
        hasPageTwo = hasPageTwo || item.pageNumber == 2;
    }

    Check(hasPageZero && hasPageTwo, "LRU replacement should evict the least recently used page");
}

void TestWaitWakesWhenChildExits()
{
    SimOS os(0, 1024, 64);
    os.NewProcess();
    os.SimFork();

    os.SimWait();
    Check(os.GetCPU() == 2, "A waiting parent should yield the CPU to its child");

    os.SimExit();
    Check(os.GetCPU() == 1, "A child exit should wake its waiting parent");
    Check(os.GetReadyQueue().empty(), "The woken parent should run immediately when the CPU is free");
}

void TestCascadingExitCleansResources()
{
    SimOS os(1, 128, 64);
    os.NewProcess();
    os.SimFork();
    os.TimerInterrupt();
    os.AccessMemoryAddress(0);
    os.DiskReadRequest(0, "child.data");

    os.SimExit();

    Check(os.GetCPU() == NO_PROCESS, "Exiting the root process should leave no process running");
    Check(os.GetDisk(0).PID == NO_PROCESS, "Cascading exit should remove active child disk work");
    Check(os.GetDiskQueue(0).empty(), "Cascading exit should remove queued child disk work");
    Check(os.GetMemory().empty(), "Cascading exit should release all descendant memory");
}

void TestInvalidOperationsFailClearly()
{
    SimOS os(1, 1024, 64);

    CheckThrows<std::logic_error>([&os] { os.TimerInterrupt(); },
                                  "CPU operations should reject an idle CPU");
    CheckThrows<std::out_of_range>([&os] { os.GetDisk(1); },
                                   "Disk access should validate the disk index");
    CheckThrows<std::out_of_range>([&os] { os.GetDisk(-1); },
                                   "Disk access should reject negative indexes");
}
} // namespace

int main()
{
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"constructor validation", TestConstructorValidation},
        {"round-robin scheduling", TestRoundRobinScheduling},
        {"disk queueing and completion", TestDiskQueueingAndCompletion},
        {"LRU page replacement", TestLeastRecentlyUsedReplacement},
        {"wait/exit synchronization", TestWaitWakesWhenChildExits},
        {"cascading resource cleanup", TestCascadingExitCleansResources},
        {"invalid operation handling", TestInvalidOperationsFailClearly},
    };

    int failures = 0;
    for (const auto& [name, test] : tests)
    {
        try
        {
            test();
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& error)
        {
            ++failures;
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }

    if (failures == 0)
    {
        std::cout << "All " << tests.size() << " tests passed.\n";
        return 0;
    }

    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
