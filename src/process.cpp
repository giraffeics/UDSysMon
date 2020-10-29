#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

CachedFunction<long> Process::cfNumCores_(LinuxParser::NumCores, 1000000000);

Process::Process(int pid)
{
    this->pid_ = pid;

    cfUtilization_.setFunction(Process::sUtilization);
    cfUtilization_.setLifetime(500);

    cfCommand_.setFunction(LinuxParser::Command);
    cfCommand_.setLifetime(1000);

    cfRam_.setFunction(LinuxParser::Ram);
    cfRam_.setLifetime(500);

    cfUser_.setFunction(LinuxParser::User);
    cfUser_.setLifetime(1000);

    cfUpTime_.setFunction(LinuxParser::UpTime);
    cfUpTime_.setLifetime(950);
}

int Process::Pid()
{
    return pid_;
}

float Process::CpuUtilization()
{
    return cfUtilization_.get(this);
}

float Process::sUtilization(Process* process)
{
    // calculate active & idle times since last sUtilization() call
    long active, idle;
    LinuxParser::Jiffies(&active, &idle, process->pid_);
    active -= process->prevActive;
    idle -= process->prevIdle;

    // return 0 utilization if this is the first sUtilization() call
    if(process->prevActive == 0 && process->prevIdle == 0)
    {
        process->prevActive = active;
        process->prevIdle = idle;
        return 0.0f;
    }

    // calculate return value
    float ret = (float) active / (float)(active + idle);

    // write back total active & idle times
    process->prevActive += active;
    process->prevIdle += idle;

    process->lastUtilization = ret;

    return ret / cfNumCores_.get();
}

string Process::Command()
{
    return cfCommand_.get(pid_);
}

string Process::Ram()
{
    return cfRam_.get(pid_);
}

string Process::User()
{
    return cfUser_.get(pid_);
}

long int Process::UpTime()
{
    return cfUpTime_.get(pid_);
}

bool Process::operator<(Process const& other) const
{
    if(other.lastUtilization == this->lastUtilization)
        return (other.pid_ > this->pid_);
    return (other.lastUtilization < this->lastUtilization);
}