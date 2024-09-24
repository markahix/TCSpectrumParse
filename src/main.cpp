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



std::string tc_jobtype(std::string filename)
{
    std::ifstream file(filename);
    std::string line;
    std::stringstream dummy;
    while (getline(file,line))
    {
        if (line.find("RUNNING AB INITIO MOLECULAR DYNAMICS") != std::string::npos)
        {
            // need to extract many sets of values
            return "BOMD";
        }
        if (line.find("RUNNING GEOMETRY OPTMIZATION") != std::string::npos)
        {
            // need to extract only one set of values
            return "OPT";
        }
        if (line.find("SINGLE POINT ENERGY CALCULATIONS") != std::string::npos)
        {
            // need to extract only one set of values
            return "SPE";
        }

    }
}

int main()
{
    // This program parses TeraChem output files into a CSV to allow for more standardized processing/plotting via python scripts.

    // 1. Verify the file is a TeraChem output file.

    // 2. Identify what kind of output it was (optimization, spe, MD, etc.)

    // 3. Identify what job settings were used (state averaged, tddft, casscf, etc.)

    // 4. Direct file to appropriate parsing function.

    // 5. Output to CSV.

    // Notes:  If multiple files, output results of each file to a csv of the same filename?

    return 0;
}