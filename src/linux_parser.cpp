#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Reads a single key/value pair from the beginning of a line in the given file
template<typename T> bool readSingleLineKey(const std::string filename, const std::string key, T* pValue)
{
  std::ifstream infile(filename);

  std::string line;
  std::string lineLabel = "";

  while(std::getline(infile, line))
  {
    std::stringstream linestream(line);
    linestream >> lineLabel;

    if(lineLabel.compare(key) == 0)
    {
      if(linestream >> *pValue)
        return true;
    }
  }

  return false;
}

// Skips several tokens so that the "nth" token can be read from the stream
std::istream& readNthToken(std::istream& in, size_t n)
{
  std::string word;
  for(size_t i=1; i<n; i++)
    in >> word;
  return in;
}

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization()
{
  float memTotal = 0.0f;
  float memFree = 0.0f;

  string line;
  string key;
  float value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> value) { // Only attempt to read one valid key-value pair
        if (key == "MemTotal") {
          memTotal = value;
        }
        else if (key == "MemFree"){
          memFree = value;
        }
      }
    }
  }
  return (memTotal - memFree) / memTotal;
}

long LinuxParser::UpTime() 
{
  double uptimeSeconds;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if(filestream.is_open()){
    if(filestream >> uptimeSeconds){
      return uptimeSeconds;
    }
  }

  return 10;
}

long LinuxParser::Jiffies()
{
  long active, idle;
  LinuxParser::Jiffies(&active, &idle);
  return active + idle;
}

long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

long LinuxParser::ActiveJiffies()
{
  long active;
  LinuxParser::Jiffies(&active, nullptr);
  return active;
}

long LinuxParser::IdleJiffies()
{
  long idle;
  LinuxParser::Jiffies(nullptr, &idle);
  return idle;
}

// read /proc/stat once to obtain processor activity information
void LinuxParser::Jiffies(long* pActive, long* pIdle)
{
  std::ifstream infile(kProcDirectory + kStatFilename);
  if(!infile.good())
    throw std::runtime_error("Could not open /proc/stat!!!");

  // seek to the line labelled "cpu"
  std::string lineLabel = "";
  while(infile.eof() == 0 && lineLabel.compare("cpu") != 0)
    infile >> lineLabel;
  if(lineLabel.compare("cpu") != 0)
    throw std::runtime_error("Invalid /proc/stat!!!");

  // read each field in /proc/stat for the "cpu" line
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  if(!(infile >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice))
    throw std::runtime_error("Invalid /proc/stat!!!");

  // return active and idle
  if(pActive != nullptr)
    *pActive = user + nice + system + irq + softirq + steal;
  if(pIdle != nullptr)
    *pIdle = idle + iowait;
}

vector<string> LinuxParser::CpuUtilization() { return {}; }

int LinuxParser::TotalProcesses()
{
  int ret = 0;
  if(readSingleLineKey(kProcDirectory + kStatFilename, "processes", &ret))
    return ret;
  return 0;
}

int LinuxParser::RunningProcesses()
{
  int ret = 0;
  if(readSingleLineKey(kProcDirectory + kStatFilename, "procs_running", &ret))
    return ret;
  return 0;
}

string LinuxParser::Command(int pid)
{
  std::ifstream infile(kProcDirectory + to_string(pid) + kCmdlineFilename);
  std::string line;
  std::getline(infile, line);
  return line;
}

string LinuxParser::Ram(int pid)
{
  long ram;
  if(readSingleLineKey(kProcDirectory + to_string(pid) + "/status", "VmSize:", &ram))
    return to_string(ram/1024);
  return "ERR";
}

string LinuxParser::Uid(int pid)
{
  string uid;
  if(readSingleLineKey(kProcDirectory + to_string(pid) + kStatusFilename, "Uid:", &uid))
    return uid;
  return "NONE";
}

string LinuxParser::User(int pid)
{
  string uid = LinuxParser::Uid(pid);
  string line;
  string user;
  string temp;

  std::ifstream infile(kPasswordPath);
  while(!infile.eof())
  {
    getline(infile, line);
    std::replace(line.begin(), line.end(), ':', ' ');
    std::stringstream linestream(line);

    linestream >> user;
    linestream >> temp;
    linestream >> temp;

    if(temp.compare(uid) == 0)
      return user;
  }

  return "NONE";
}

long LinuxParser::NumCores()
{
  std::ifstream infile(kProcDirectory + kCpuinfoFilename);

  long numCores = 0;

  while(!infile.eof())
  {
    string line;
    getline(infile, line);
    if(line.rfind("processor", 0) == 0)
      numCores++;
  }

  return numCores;
}

long LinuxParser::UpTime(int pid)
{
  std::ifstream infile(kProcDirectory + to_string(pid) + kStatFilename);
  long startTime = 0;
  readNthToken(infile, 22) >> startTime;
  return (UpTime() - startTime / sysconf(_SC_CLK_TCK));  
}

void LinuxParser::Jiffies(long* active, long* idle, int pid)
{
  double uptime_s;
  std::ifstream uptime_infile(kProcDirectory + kUptimeFilename);
  uptime_infile >> uptime_s;
  uptime_infile.close();

  long utime, stime, cutime, cstime, hz, uptime_ms;
  hz = sysconf(_SC_CLK_TCK);
  uptime_ms = uptime_s * 1000;

  std::ifstream stat_infile(kProcDirectory + to_string(pid) + kStatFilename);
  readNthToken(stat_infile, 14) >> utime >> stime >> cutime >> cstime;

  long used_ms = 1000 * (utime + stime + cutime + cstime) / hz;

  if(active != nullptr)
    *active = used_ms;

  if(idle != nullptr)
    *idle = uptime_ms - used_ms;
}