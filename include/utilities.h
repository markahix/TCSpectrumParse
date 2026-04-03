#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <experimental/filesystem>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <locale>
#include <vector>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <iomanip>
#include <ctime>
#include <set>

extern bool DEBUGGING;
extern bool COMBINE_FILES;

void debug(std::string printstring);
std::vector<std::string> get_file_list(int argc, char** argv);
void silent_shell(const char* cmd);
double ev_to_nm(double ev);
std::vector<double> gauss(double a, double x_naught);
std::vector<double> gauss(double a, double x_naught, std::vector<double> ev_ranges);

#endif