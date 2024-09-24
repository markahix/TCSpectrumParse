#include "classes.h"

JobSettings::JobSettings(std::string filename)
{
    inputfile = filename;
    runtype = "";
    calctype = "";
    std::ifstream file(filename);
    std::string line;
    while (getline(file,line))
    {
        // IDENTIFY RUNTYPE
        if (line.find("RUNNING AB INITIO MOLECULAR DYNAMICS") != std::string::npos)
        {
            // need to extract many sets of values
            runtype = "BOMD";
        }
        if (line.find("RUNNING GEOMETRY OPTMIZATION") != std::string::npos)
        {
            // need to extract only one set of values
            runtype = "OPT";
        }
        if (line.find("SINGLE POINT ENERGY CALCULATIONS") != std::string::npos)
        {
            // need to extract only one set of values
            runtype = "SPE";
        }
        // IDENTIFY CALCTYPE
        if (line.find("CAS Parameters") != std::string::npos)
        {
            // I'm assuming CASSCF calculation will be the only one listing out CAS parameters...
            calctype = "CASSCF";
        }
        if (line.find("DFT Functional requested") != std::string::npos)
        {
            calctype = "TDDFT";
        }

    }
    if (calctype == "" || runtype == "")
    {
        std::cout << "Unable to parse input file for calculation/run types:" <<std::endl;
        std::cout << "\t" << filename << std::endl;
    }
}

JobSettings::~JobSettings()
{
}
