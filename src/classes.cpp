#include "classes.h"
#include "utilities.h"

JobSettings::JobSettings(std::string filename)
{
    inputfile = filename;
    runtype = "";
    calctype = "";

    std::stringstream buffer;
    std::ifstream file(filename);
    std::string line;
    while (getline(file,line))
    {
        buffer.str("");
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
            calctype = "SA-CASSCF";
        }
        if (line.find("DFT Functional requested") != std::string::npos)
        {
            calctype = "TDDFT";
        }
    }
    
    //Report failure
    if (calctype == "" || runtype == "")
    {
        buffer.str("");
        buffer << "Unable to parse input file for calculation/run types:" << std::endl;
        buffer << "\t" << filename << std::endl;
        debug(buffer.str());
    }

    // Report results of parse.
    if (calctype == "TDDFT")
    {
        buffer << filename << " is using TDDFT." << std::endl;
        debug(buffer.str());
    }
    else if (calctype == "SA-CASSCF")
    {
        buffer << filename << " is using CASSCF." << std::endl;
        debug(buffer.str());
    }
    if (runtype == "SPE")
    {
        buffer << filename << " calculation type is QM/MM single point energy." << std::endl;
        debug(buffer.str());
    }
    else if (runtype == "OPT")
    {
        buffer << filename << " calculation type is QM/MM optimization." << std::endl;
        debug(buffer.str());
    }
    else if (runtype == "BOMD")
    {
        buffer << filename << " calculation type is QM/MM-MD (Born-Oppenheimer Molecular Dynamics)." << std::endl;
        debug(buffer.str());
    }
}

JobSettings::~JobSettings()
{
}
