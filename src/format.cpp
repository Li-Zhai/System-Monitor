#include <string>

#include "format.h"

using std::string;
using std::to_string;

string Format::ElapsedTime(long seconds) { 
  int h = seconds / 3600;
  int m = seconds % 3600 / 60;
  int s = seconds % 60;
  
  string hour = (h < 10) ? "0" + to_string(h) : to_string(h);
  string minute = (m < 10) ? "0" + to_string(m) : to_string(m);
  string second = (s < 10) ? "0" + to_string(s) : to_string(s);
  
  return hour + ":" + minute + ":" + second; 
}