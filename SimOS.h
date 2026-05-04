// Jason Parmar

#ifndef SIMOS_H
#define SIMOS_H

#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

struct FileReadRequest
{
    int  PID{0};
    std::string fileName{""};
};

struct MemoryItem
{
    unsigned long long pageNumber;
    unsigned long long frameNumber;
    int PID;
};

using MemoryUsage = std::vector<MemoryItem>;

constexpr int NO_PROCESS{ 0 };

class SimOS
{
public:
    SimOS(int numberOfDisks, unsigned long long amountOfRAM, unsigned int pageSize);

    void NewProcess();
    void SimFork();
    void SimExit();
    void SimWait();
    void TimerInterrupt();
    void DiskReadRequest(int diskNumber, std::string fileName);
    void DiskJobCompleted(int diskNumber);
    void AccessMemoryAddress(unsigned long long address);

    int GetCPU();
    std::deque<int> GetReadyQueue();
    MemoryUsage GetMemory();
    FileReadRequest GetDisk(int diskNumber);
    std::deque<FileReadRequest> GetDiskQueue(int diskNumber);

private:
    struct Process
    {
        int pid{0};
        int parentPid{0};
        std::vector<int> children;
        bool waiting{false};
        bool zombie{false};
    };

    struct Frame
    {
        unsigned long long pageNumber{0};
        unsigned long long frameNumber{0};
        int pid{0};
        unsigned long long lastUsed{0};
        bool used{false};
    };

    struct Disk
    {
        FileReadRequest current;
        std::deque<FileReadRequest> queue;
    };

    int cpu;
    int nextPid;
    unsigned int pageSizeValue;
    unsigned long long frameCount;
    unsigned long long timer;

    std::deque<int> readyQueue;
    std::vector<Disk> disks;
    std::vector<Frame> frames;
    std::unordered_map<int, Process> processes;

    void CheckCPU();
    void CheckDisk(int diskNumber);
    void ScheduleCPU();
    void RemoveFromReadyQueue(int pid);
    void RemoveFromDisks(int pid);
    void RemoveMemoryOfProcess(int pid);
    void RemoveChildFromParent(int childPid);
    void CascadeTerminateChildren(int pid);
    void FullyRemoveProcess(int pid);
    bool HasZombieChild(int pid, int& zombiePid);
};

#endif