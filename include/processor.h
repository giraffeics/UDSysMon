#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <cached_function.h>

class Processor {
    public:
        Processor();
        float Utilization();

    private:
        static float sUtilization(Processor* processor);
        CachedFunction<float, Processor*> cfUtilization_ = {};

        long prevActive = 0;
        long prevIdle = 0;
};

#endif