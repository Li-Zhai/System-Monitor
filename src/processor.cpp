#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() { 
  return (float(LinuxParser::ActiveJiffies()) + float(LinuxParser::IdleJiffies())) / float(LinuxParser::Jiffies());
}