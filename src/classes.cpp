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
        buffer << "Unable to parse input file for calculation/run types:";
        buffer << "\t" << filename;
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

//
/*
struct Excitation
{
    std::string transition_name;
    double energy;
    double oscillator;
};

class OverallDataStructure
{
    public:
        OverallDataStructure();
        ~OverallDataStructure();
        // each datastructure is composed of a sequence of timesteps, frames, or individual files that get parsed into their respective excitations.
        std::vector<TimeStep> timesteps;
        
        // We can simply add any number of files to the mix, with each one being parsed into the datastructure as we go.
        void AddInputFile(std::string filename);
        void WriteOutputFile(std::string filename);
        void AddTimestep();
        void AddExcitation(std::string transition_name, double ene, double osc_str);
};

*/
// TimeStep class functions
TimeStep::TimeStep()
{
    // std::vector<Excitation> excitations;
    excitations = {};
}

TimeStep::~TimeStep()
{
    
}

void TimeStep::AddExcitation(std::string transition_name, double ene, double osc_str)
{
    excitations[transition_name] = {ene,osc_str};
}

// OverallDataStructure class functions
OverallDataStructure::OverallDataStructure()
{
    min_nm = 200;
    max_nm = 800;
    known_transition_names = {};
    // std::vector<TimeStep> timesteps;
    timesteps = {};
    std::stringstream buffer;
    int file_count=0;
    do
    {
        file_count++;
        buffer.str("");
        buffer << "ConsolidatedSpectralData_" << std::setw(4) << std::setfill('0') << file_count << ".csv";
    } while (std::experimental::filesystem::exists(buffer.str()));
    
    output_csv_filename = buffer.str();
    buffer.str("");
    buffer << "ConsolidatedSpectralData_" << std::setw(4) << std::setfill('0') << file_count << ".png";
    output_png_filename = buffer.str();
}

OverallDataStructure::~OverallDataStructure()
{

}

void OverallDataStructure::UpdateMinMaxEnergyRange()
{
    // Iterate through all timesteps, get min and max values for excitation energies
}

void OverallDataStructure::ParseTDDFTBlock(std::vector<std::string> block)
{
    for (int i=0; i < block.size(); i++)
    {
        std::string line=block[i];
        if (line.find("  Final Excited State Results:") != std::string::npos)
        {
        std::string dummy, excite, osci;
        std::stringstream buffer;
        buffer.str(block[i+4]);
        buffer >> dummy >> dummy >> excite >> osci;
        
        // excitations.push_back(stof(excite));
        // oscillators.push_back(stof(osci));
        debug("Excitation Energy: " + excite);
        debug("Oscillator: "+ osci);
        }
    }
}

void OverallDataStructure::ParseSACASSCFBlock(std::vector<std::string> block)
{

}

void OverallDataStructure::ParseBlock(std::vector<std::string> block, std::string calctype)
{
    if (calctype == "TDDFT")
    {
        ParseTDDFTBlock(block);
    }
    if (calctype == "SA-CASSCF")
    {
        ParseSACASSCFBlock(block);
    }
}

void OverallDataStructure::ParseOPT(JobSettings current)
{
        std::string line;
        std::stringstream dummy;
        std::vector<std::string> block;
        std::ifstream file(current.inputfile);
        while (getline(file,line))
        {
            if (line.find("Final Excited State Results:") != std::string::npos)
            {
                // We only want the final optimized results, not the entire process.
                block.clear();
            }
            block.push_back(line);
        }
        file.close();
        ParseBlock(block, current.calctype);
}

void OverallDataStructure::ParseSPE(JobSettings current)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(current.inputfile);
    while (getline(file,line))
    {
        block.push_back(line);
    }
    ParseBlock(block,current.calctype);
    file.close();
}

void OverallDataStructure::ParseBOMD(JobSettings current)
{

}

void OverallDataStructure::AddInputFile(std::string filename)
{
    // Identify calctype and runtype from the filename
    JobSettings current(filename);

    // parse the file accordingly.
    if (current.runtype == "OPT")
    {
        ParseOPT(filename);
    }

    if (current.runtype == "BOMD")
    {
        ParseBOMD(filename);
    }
    
    if (current.runtype == "SPE")
    {
        ParseSPE(filename);
    }
}

void OverallDataStructure::GenerateSpectralData()
{
    // Generate energies array (in nm and eV for ease of CSV output and later parsing.)
    for (TimeStep ts : timesteps)
    {
        // Iterate through all timesteps, generating spectral data for each transition
    }
}
void OverallDataStructure::WriteOutputFile()
{
    for (TimeStep ts: timesteps)
    {
        // each timestep needs the energy values 
    }
}

void OverallDataStructure::AddTimestep()
{
    timesteps.push_back(TimeStep());
}

void OverallDataStructure::AddExcitation(std::string transition_name, double ene, double osc_str)
{
    timesteps.back().AddExcitation(transition_name, ene, osc_str);
}

void OverallDataStructure::PlotSpectra()
{
    // include spans for visible, IR, UV, etc. on the main plot
    // Set x limits to the min_nm and max_nm values in this class.
}