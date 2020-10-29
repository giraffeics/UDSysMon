#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <cached_function.h>


class Process {
    public:
        Process(int pid);

        int Pid();
        std::string User();
        std::string Command();
        float CpuUtilization();
        std::string Ram();
        long int UpTime();
        bool operator<(Process const& a) const;

    private:
        int pid_;

        CachedFunction<float, Process*> cfUtilization_ = {};
        CachedFunction<std::string, int> cfCommand_ = {};
        CachedFunction<std::string, int> cfRam_ = {};
        CachedFunction<std::string, int> cfUser_ = {};
        CachedFunction<long int, int> cfUpTime_ = {};
        static CachedFunction<long> cfNumCores_;

        static float sUtilization(Process* process);
        long prevActive = 0;
        long prevIdle = 0;

        float lastUtilization = 0.0f;
};

#endif