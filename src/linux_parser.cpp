#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <unordered_map>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// BONUS: Update this to use std::filesystem
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

float LinuxParser::MemoryUtilization() {
  string line, key;
  float value;
  unordered_map<string, float> dict;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        dict[key] = value;
      }
    }
  }
  return dict[filterMemFreeString] / dict[filterMemTotalString];
}

long LinuxParser::UpTime() {
  string line;
  float uptime, idle;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idle;
  }
  return uptime;
}

long LinuxParser::Jiffies() {
  vector<string> utilization = LinuxParser::CpuUtilization();
  long res;
  for (auto i : utilization) {
    res += std::stoi(i);
  }
  return res;
}

long LinuxParser::ActiveJiffies(int pid) {
  string line, tmp_string;
  float t1, t2, t3, t4;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int i = 0; i <= 12; i++) {
      linestream >> tmp_string;
    }
    linestream >> t1 >> t2 >> t3 >> t4;
  }
  return t1 + t2 + t3 + t4;
}

long LinuxParser::ActiveJiffies() {
  vector<string> utilization = LinuxParser::CpuUtilization();
  long res;
  for (unsigned int i = 0; i < utilization.size(); i++) {
    if (i == 3 or i == 4) { continue; }
    res += std::stoi(utilization[i]);
  }
  return res;
}

long LinuxParser::IdleJiffies() { 
  vector<string> utilization = LinuxParser::CpuUtilization();
  return std::stoi(utilization[3]) + std::stoi(utilization[4]);
}

vector<string> LinuxParser::CpuUtilization() { 
  vector<string> utilization;
  string line, tmp_string, tmp_value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> tmp_string;
    while (linestream >> tmp_value) {
      utilization.emplace_back(tmp_value);
    }
  }
  return utilization; 
}

int LinuxParser::TotalProcesses() {
  string line, key;
  float value;
  unordered_map<string, float> dict;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        dict[key] = value;
      }
    }
  }
  return dict[filterProcesses];
}

int LinuxParser::RunningProcesses() {
  string line, key;
  float value;
  unordered_map<string, float> dict;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        dict[key] = value;
      }
    }
  }
  return dict[filterRunningProcesses];
}

string LinuxParser::Command(int pid) {
  string line, cmdline;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      cmdline += line + " ";
    };
  }
  if (cmdline.length() > 40) {  // clip the cmdline to max length of 40
    cmdline = cmdline.substr(0, 40) + "...";
  }
  return cmdline;
}

string LinuxParser::Ram(int pid) {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == filterProcMem) {
        break;
      }
    }
  }
  // add for precision, and just for fun, also meke the display looks cleaner
  string int_part = to_string(int(stof(value) / 1024));
  string else_part = to_string(int(stof(value)) % 1024);
  int len = int_part.length();
  return int_part + "." + else_part.substr(0, 6 - len);;
}

string LinuxParser::Uid(int pid) {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == filterUID) {
        break;
      }
    }
  }
  return value;
}

string LinuxParser::User(int pid) {
  string Uid = LinuxParser::Uid(pid);
  string line, name, uid;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::istringstream linestream(line);
      linestream >> name >> uid;
      if (uid == Uid) {
        break;
      }
    }
  }
  return name;
}

long LinuxParser::UpTime(int pid) {
  string line, tmp_string, value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> value >> tmp_string >> tmp_string;
    for (int i = 0; i <= 18; i++) {
      linestream >> value;
    }
  }
  // revised agter getting the review
  return UpTime() - stol(value)/sysconf(_SC_CLK_TCK);;
}
