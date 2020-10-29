#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

System::System()
{
    cMemoryUtilization_.setFunction(LinuxParser::MemoryUtilization);
    cMemoryUtilization_.setLifetime(500);

    cUptime_.setFunction(LinuxParser::UpTime);
    cUptime_.setLifetime(950);

    cProcesses_.setFunction(System::sProcesses);
    cProcesses_.setLifetime(500);
}

Processor& System::Cpu()
{
    return cpu_;
}

vector<Process>& System::Processes()
{
    return *(cProcesses_.get(this));
}

long binSearchPIDIndex(std::vector<Process>& processes, int pid, long begin, long end)
{
    long low = begin;
    long high = end;
    long i = (low+high)/2;

    while(high >= low)
    {
        int curPid = processes[i].Pid();

        if(curPid > pid)
        {
            high = i-1;
            i = (low+high)/2;
        }
        else if(curPid < pid)
        {
            low = i+1;
            i = (low+high)/2;
        }
        else
            return i;
    }

    return -1;
}

bool processComparePID(Process& a, Process& b)
{
    return (a.Pid() < b.Pid());
}

bool processCompareCPU(Process& a, Process& b)
{
    float aCpu = a.CpuUtilization();
    float bCpu = b.CpuUtilization();
    if(aCpu == bCpu)
        return (a.Pid() < b.Pid());
    return (aCpu > bCpu);
}

std::vector<Process>* System::sProcesses(System* system)
{
    std::vector<int> pids = LinuxParser::Pids();

    // Add any missing PIDs to the list
    std::sort(system->processes_.begin(), system->processes_.end(), processComparePID);
    auto lastIndex = system->processes_.size() - 1;
    for(int pid : pids)
    {
        long index = binSearchPIDIndex(system->processes_, pid, 0, lastIndex);

        if(index < 0)
            system->processes_.push_back(Process(pid));
    }

    // Sort into display order
    std::sort(system->processes_.begin(), system->processes_.end(), processCompareCPU);

    return &(system->processes_);
}

std::string System::Kernel()
{
    if(!kernelKnown_)
        kernel_ = LinuxParser::Kernel();

    return kernel_;
}

float System::MemoryUtilization()
{ 
    return cMemoryUtilization_.get();
}

std::string System::OperatingSystem()
{
    if(!osKnown_)
        os_ = LinuxParser::OperatingSystem();

    return os_;
}

int System::RunningProcesses()
{
    return LinuxParser::RunningProcesses();
}

int System::TotalProcesses()
{
    return LinuxParser::TotalProcesses();
}

long int System::UpTime()
{
    return cUptime_.get();
}