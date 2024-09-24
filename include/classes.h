#ifndef CLASSES_H
#define CLASSES_H

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

class JobSettings
{
private:
    std::string inputfile;
    std::string runtype;
    std::string calctype;
    
    /* data */
public:
    JobSettings(std::string filename);
    ~JobSettings();
};

#endif