#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <time.h>

#include "process.h"
#include "processor.h"
#include "cached_function.h"
#include "linux_parser.h"

class System {
 public:
  Processor& Cpu();
  std::vector<Process>& Processes();
  float MemoryUtilization();
  long UpTime();
  int TotalProcesses();
  int RunningProcesses();
  std::string Kernel();
  std::string OperatingSystem();
  System();

 private:
  Processor cpu_ = {};
  std::vector<Process> processes_ = {};

  CachedFunction<float> cMemoryUtilization_ = {};
  CachedFunction<long> cUptime_ = {};
  CachedFunction<std::vector<Process>*, System*> cProcesses_ = {};

  static std::vector<Process>* sProcesses(System* system);

  bool osKnown_ = false;
  std::string os_;
  bool kernelKnown_ = false;
  std::string kernel_;
};

#endif