#include "ncurses_display.h"
#include "system.h"
#include "process.h"
#include "linux_parser.h"
#include <iostream>

int main() {
  System system;
  NCursesDisplay::Display(system);
}