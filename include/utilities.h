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
double gauss_at_energy(double osc, double energy_peak, double energy_eval);
std::string GetRunType(std::string filename);
std::string GetCalcType(std::string filename);

std::string Combine_CSVs(std::vector<std::string> csv_filenames);
#endif