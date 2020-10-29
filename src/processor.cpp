#include "processor.h"
#include "linux_parser.h"

Processor::Processor()
{
    this->cfUtilization_.setFunction(sUtilization);
    this->cfUtilization_.setLifetime(500);
}

float Processor::Utilization()
{
    return this->cfUtilization_.get(this);
}

float Processor::sUtilization(Processor* processor)
{
    // calculate active & idle times since last sUtilization() call
    long active, idle;
    LinuxParser::Jiffies(&active, &idle);
    active -= processor->prevActive;
    idle -= processor->prevIdle;

    // return 0 utilization if this is the first sUtilization() call
    if(processor->prevActive == 0 && processor->prevIdle == 0)
    {
        processor->prevActive = active;
        processor->prevIdle = idle;
        return 0.0f;
    }

    // calculate return value
    float ret = (float) active / (float)(active + idle);

    // write back total active & idle times
    processor->prevActive += active;
    processor->prevIdle += idle;

    return ret;
}