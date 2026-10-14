#include "SimOS.h"

SimOS::SimOS(int numberOfDisks, unsigned long long amountOfRAM, unsigned int pageSize)
    : cpu(NO_PROCESS),
      nextPid(1),
      pageSizeValue(pageSize),
      frameCount(0),
      timer(0)
{
    if (numberOfDisks < 0)
    {
        throw std::invalid_argument("Number of disks cannot be negative");
    }

    if (pageSize == 0)
    {
        throw std::invalid_argument("Page size must be greater than zero");
    }

    frameCount = amountOfRAM / pageSize;
    disks.resize(static_cast<std::size_t>(numberOfDisks));
    frames.reserve(static_cast<std::size_t>(frameCount));

    for (unsigned long long i = 0; i < frameCount; ++i)
    {
        Frame frame;
        frame.frameNumber = i;
        frames.push_back(frame);
    }
}

void SimOS::CheckCPU()
{
    if (cpu == NO_PROCESS)
    {
        throw std::logic_error("CPU is idle");
    }
}

void SimOS::CheckDisk(int diskNumber) const
{
    if (diskNumber < 0 || diskNumber >= static_cast<int>(disks.size()))
    {
        throw std::out_of_range("Disk does not exist");
    }
}

void SimOS::ScheduleCPU()
{
    if (cpu == NO_PROCESS && !readyQueue.empty())
    {
        cpu = readyQueue.front();
        readyQueue.pop_front();
    }
}

void SimOS::NewProcess()
{
    Process process;
    process.pid = nextPid;
    processes[nextPid] = process;

    if (cpu == NO_PROCESS)
    {
        cpu = nextPid;
    }
    else
    {
        readyQueue.push_back(nextPid);
    }

    ++nextPid;
}

void SimOS::SimFork()
{
    CheckCPU();

    Process child;
    child.pid = nextPid;
    child.parentPid = cpu;

    processes[nextPid] = child;
    processes[cpu].children.push_back(nextPid);

    readyQueue.push_back(nextPid);
    ++nextPid;
}

void SimOS::RemoveFromReadyQueue(int pid)
{
    std::deque<int> updated;

    while (!readyQueue.empty())
    {
        int currentPid = readyQueue.front();
        readyQueue.pop_front();

        if (currentPid != pid)
        {
            updated.push_back(currentPid);
        }
    }

    readyQueue = updated;
}

void SimOS::RemoveFromDisks(int pid)
{
    for (Disk& disk : disks)
    {
        if (disk.current.PID == pid)
        {
            disk.current = FileReadRequest{};
        }

        std::deque<FileReadRequest> updated;

        while (!disk.queue.empty())
        {
            FileReadRequest request = disk.queue.front();
            disk.queue.pop_front();

            if (request.PID != pid)
            {
                updated.push_back(request);
            }
        }

        disk.queue = updated;

        if (disk.current.PID == NO_PROCESS && !disk.queue.empty())
        {
            disk.current = disk.queue.front();
            disk.queue.pop_front();
        }
    }
}

void SimOS::RemoveMemoryOfProcess(int pid)
{
    for (Frame& frame : frames)
    {
        if (frame.used && frame.pid == pid)
        {
            frame.used = false;
            frame.pid = 0;
            frame.pageNumber = 0;
            frame.lastUsed = 0;
        }
    }
}

void SimOS::RemoveChildFromParent(int childPid)
{
    if (processes.find(childPid) == processes.end())
    {
        return;
    }

    int parentPid = processes[childPid].parentPid;

    if (parentPid == 0 || processes.find(parentPid) == processes.end())
    {
        return;
    }

    std::vector<int>& children = processes[parentPid].children;
    std::vector<int> updated;

    for (int pid : children)
    {
        if (pid != childPid)
        {
            updated.push_back(pid);
        }
    }

    children = updated;
}

void SimOS::FullyRemoveProcess(int pid)
{
    RemoveFromReadyQueue(pid);
    RemoveFromDisks(pid);
    RemoveMemoryOfProcess(pid);
    RemoveChildFromParent(pid);

    if (cpu == pid)
    {
        cpu = NO_PROCESS;
    }

    processes.erase(pid);
}

void SimOS::CascadeTerminateChildren(int pid)
{
    if (processes.find(pid) == processes.end())
    {
        return;
    }

    std::vector<int> children = processes[pid].children;

    for (int childPid : children)
    {
        CascadeTerminateChildren(childPid);
        FullyRemoveProcess(childPid);
    }

    processes[pid].children.clear();
}

bool SimOS::HasZombieChild(int pid, int& zombiePid)
{
    if (processes.find(pid) == processes.end())
    {
        return false;
    }

    for (int childPid : processes[pid].children)
    {
        if (processes.find(childPid) != processes.end() && processes[childPid].zombie)
        {
            zombiePid = childPid;
            return true;
        }
    }

    return false;
}

void SimOS::SimExit()
{
    CheckCPU();

    int exitingPid = cpu;
    int parentPid = processes[exitingPid].parentPid;

    CascadeTerminateChildren(exitingPid);
    RemoveMemoryOfProcess(exitingPid);
    RemoveFromDisks(exitingPid);
    RemoveFromReadyQueue(exitingPid);

    cpu = NO_PROCESS;

    if (parentPid != 0 &&
        processes.find(parentPid) != processes.end() &&
        processes[parentPid].waiting)
    {
        processes[parentPid].waiting = false;
        FullyRemoveProcess(exitingPid);

        if (cpu == NO_PROCESS)
        {
            cpu = parentPid;
        }
        else
        {
            readyQueue.push_back(parentPid);
        }
    }
    else if (parentPid != 0 && processes.find(parentPid) != processes.end())
    {
        processes[exitingPid].zombie = true;
    }
    else
    {
        FullyRemoveProcess(exitingPid);
    }

    ScheduleCPU();
}

void SimOS::SimWait()
{
    CheckCPU();

    int zombiePid = 0;

    if (HasZombieChild(cpu, zombiePid))
    {
        FullyRemoveProcess(zombiePid);
        return;
    }

    if (processes[cpu].children.empty())
    {
        return;
    }

    int waitingPid = cpu;
    processes[waitingPid].waiting = true;
    cpu = NO_PROCESS;

    ScheduleCPU();
}

void SimOS::TimerInterrupt()
{
    CheckCPU();

    readyQueue.push_back(cpu);
    cpu = NO_PROCESS;
    ScheduleCPU();
}

void SimOS::DiskReadRequest(int diskNumber, std::string fileName)
{
    CheckCPU();
    CheckDisk(diskNumber);

    FileReadRequest request;
    request.PID = cpu;
    request.fileName = fileName;

    if (disks[diskNumber].current.PID == NO_PROCESS)
    {
        disks[diskNumber].current = request;
    }
    else
    {
        disks[diskNumber].queue.push_back(request);
    }

    cpu = NO_PROCESS;
    ScheduleCPU();
}

void SimOS::DiskJobCompleted(int diskNumber)
{
    CheckDisk(diskNumber);

    if (disks[diskNumber].current.PID != NO_PROCESS)
    {
        readyQueue.push_back(disks[diskNumber].current.PID);
        disks[diskNumber].current = FileReadRequest{};
    }

    if (!disks[diskNumber].queue.empty())
    {
        disks[diskNumber].current = disks[diskNumber].queue.front();
        disks[diskNumber].queue.pop_front();
    }

    ScheduleCPU();
}

void SimOS::AccessMemoryAddress(unsigned long long address)
{
    CheckCPU();

    unsigned long long pageNumber = address / pageSizeValue;
    ++timer;

    for (Frame& frame : frames)
    {
        if (frame.used && frame.pid == cpu && frame.pageNumber == pageNumber)
        {
            frame.lastUsed = timer;
            return;
        }
    }

    for (Frame& frame : frames)
    {
        if (!frame.used)
        {
            frame.used = true;
            frame.pid = cpu;
            frame.pageNumber = pageNumber;
            frame.lastUsed = timer;
            return;
        }
    }

    if (frames.empty())
    {
        return;
    }

    unsigned long long lruIndex = 0;

    for (unsigned long long i = 1; i < frames.size(); ++i)
    {
        if (frames[i].lastUsed < frames[lruIndex].lastUsed)
        {
            lruIndex = i;
        }
    }

    frames[lruIndex].used = true;
    frames[lruIndex].pid = cpu;
    frames[lruIndex].pageNumber = pageNumber;
    frames[lruIndex].lastUsed = timer;
}

int SimOS::GetCPU() const
{
    return cpu;
}

std::deque<int> SimOS::GetReadyQueue() const
{
    return readyQueue;
}

MemoryUsage SimOS::GetMemory() const
{
    MemoryUsage memory;

    for (const Frame& frame : frames)
    {
        if (frame.used)
        {
            MemoryItem item;
            item.pageNumber = frame.pageNumber;
            item.frameNumber = frame.frameNumber;
            item.PID = frame.pid;

            memory.push_back(item);
        }
    }

    return memory;
}

FileReadRequest SimOS::GetDisk(int diskNumber) const
{
    CheckDisk(diskNumber);
    return disks[diskNumber].current;
}

std::deque<FileReadRequest> SimOS::GetDiskQueue(int diskNumber) const
{
    CheckDisk(diskNumber);
    return disks[diskNumber].queue;
}
