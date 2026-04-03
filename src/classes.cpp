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

void TimeStep::GenerateSpectralData(std::vector<double> ev_ranges)
{
    for (std::map<std::string, Excitation>::iterator excite = excitations.begin(); excite != excitations.end(); excite++)
    {
        std::string t_name = excite->first;
        double osc = excite->second.oscillator;
        double ene = excite->second.energy;
        spectral_data[t_name] = gauss(osc, ene, ev_ranges);
    }    
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
    for (TimeStep ts : timesteps)
    {
        // Iterate through all timesteps, generating spectral data for each transition
        for (std::map<std::string, Excitation>::iterator exc = ts.excitations.begin(); exc != ts.excitations.end(); exc++)
        {
            // iterate through each excitation, identifying 
            double ene = exc->second.energy;
            if (min_nm > ev_to_nm(ene))
            {
                min_nm = ev_to_nm(ene);
            }
            if (max_nm < ev_to_nm(ene))
            {
                max_nm = ev_to_nm(ene);
            }
        }
    }

    // adding 20% of the min-max nm range to the ends of the spectrum.
    double nm_range = (max_nm - min_nm) * .2; 
    min_nm = std::round(min_nm - nm_range);
    max_nm = std::round(max_nm + nm_range);    
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
            AddExcitation("TDDFT", stof(excite), stof(osci));
        }
    }
}

void OverallDataStructure::ParseSACASSCFBlock(std::vector<std::string> block)
{
    std::map<std::string, double> state_energies = {};
    std::map<std::string, int> mult_count = {{"singlet",-1},{"doublet",-1},{"triplet",-1},{"quartet",-1}}; // Start at -1 so that the first encountered multiplicity state is S0, D0, T0, etc.
    std::map<std::string,std::string> multiplicity_prefixes = {{"singlet","S"},{"doublet","D"},{"triplet","T"},{"quartet","Q"}};
    std::vector<std::string> state_names = {};
    std::string current_mult = "S";
    std::map<std::string,double> excitation_energies = {};

    for (int i = 0; i < block.size(); i++)
    {
        std::string line = block[i];
        if (line.find("Root   Mult.   Total Energy (a.u.)   Ex. Energy (a.u.)     Ex. Energy (eV)") != std::string::npos) // Obtain all state energies and the different state names.
        {
            // skip the line of dashes:  i++; twice so that the next line is the first root energy
            i++;
            i++;
            line = block[i];
            while (line != "")
            {
                std::string state;
                std::string mult;
                double energy;
                std::stringstream buffer;

                // get state number, multiplicity, and energy value
                buffer.str(line);
                buffer >> state >> mult >> energy;
                buffer.str("");

                // update count of multiplicities
                mult_count[mult]++;

                // generate state name from this line (S0, T2, etc.)
                std::stringstream s_name;
                s_name.str("");
                s_name << multiplicity_prefixes[mult];
                s_name << mult_count[mult];

                // assign state energy for this state.
                state_energies[s_name.str()] = energy;

                // save the state name to the vector list.
                state_names.push_back(s_name.str());
                i++;
                line = block[i];
            }

            // once we're clear of the root energies, we need to get the excitation energies between states (no multiplicity changes allowed).
            // Also need transition names:
            std::vector<std::string> transition_names = {};
            
            for (std::map<std::string, int>::iterator mc = mult_count.begin(); mc != mult_count.end(); mc++)
            {
                std::string key = mc->first;
                int count = mc->second;
                // iterate through all integers in the count.
                for (int j = 0; j < count-1; j++)
                {   
                    // get state name 1 for root energy 1
                    std::stringstream sname_1;
                    sname_1.str("");
                    sname_1 << multiplicity_prefixes[key] << j;
                    double ene_a = state_energies[sname_1.str()];

                    for (int k = j+1; k < count; k++)
                    {
                        // get state name 2 for root energy 2
                        std::stringstream sname_2;
                        sname_2.str("");
                        sname_2 << multiplicity_prefixes[key] << k;
                        double ene_b = state_energies[sname_2.str()];

                        // get transition name from both statenames
                        std::stringstream t_name;
                        t_name.str("");
                        t_name << sname_1.str() << "->" << sname_2.str();
                        transition_names.push_back(t_name.str());
                        excitation_energies[t_name.str()] = (ene_b - ene_a) * 27.211324570273; // convert from Hartrees to eV
                    }
                }
            }
        }
        if (line.find("Singlet state electronic transitions")!= std::string::npos)
        {
            current_mult = "S";
        }
        if (line.find("Triplet state electronic transitions")!= std::string::npos)
        {
            current_mult = "T";
        }

        if (line.find("Transition      Tx        Ty        Tz       |T|    Osc. (a.u.)") != std::string::npos)
        {
            // Skip dashes.
            i++;
            i++;
            line = block[i];
            while (line != "")
            {
                int from_state;
                int to_state;
                std::string transition_name;
                std::string junk;
                double osc_str;
                std::stringstream buffer;

                // Get transition name, oscillator strength from line.  Energy is already stored above.
                buffer.str(line);
                buffer >> from_state >> junk >> to_state >> junk >> junk >> junk >> junk >> osc_str;
                buffer.str("");

                std::stringstream t_name;
                t_name.str("");
                t_name << current_mult << from_state-1 <<  "->" << current_mult << to_state-1;
                double curr_energy = excitation_energies[t_name.str()];
                AddExcitation(t_name.str(), curr_energy, osc_str);
                i++;
                line = block[i];
            }
        }
    }
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
        ParseBlock(block, current.calctype);
        AddTimestep();
        file.close();
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
    AddTimestep();
    file.close();
}

void OverallDataStructure::ParseBOMD(JobSettings current)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(current.inputfile);
    std::cout << "Opened " << current.inputfile << " for parsing." << std::endl;
    int n_frames_parsed = 0;
    while (getline(file,line))
    {
        block.push_back(line);
        if (line.find("MD STEP") != std::string::npos)
        {
            ParseBlock(block,current.calctype);
            block.clear();
            AddTimestep();
        }
        n_frames_parsed++;
        std::cout << "Parsed a frame. " << n_frames_parsed << std::endl;
    }
    ParseBlock(block, current.calctype);
    AddTimestep();
    file.close();
    std::cout << "Finished parsing the file." << std::endl;
}

void OverallDataStructure::AddInputFile(std::string filename)
{
    // Identify calctype and runtype from the filename
    JobSettings current(filename);

    std::cout << "created 'current' with " << filename << std::endl;
    // parse the file accordingly.
    if (current.runtype == "OPT")
    {
        ParseOPT(current);
    }

    if (current.runtype == "BOMD")
    {
        ParseBOMD(current);
    }
    
    if (current.runtype == "SPE")
    {
        ParseSPE(current);
    }
}

void OverallDataStructure::GenerateSpectralData()
{
    UpdateMinMaxEnergyRange();
    ev_ranges = {};
    nm_ranges = {};
    
    // Generate energies array (in nm and eV for ease of CSV output and later parsing.)
    for (int i = (int)min_nm; i < (int)max_nm; i++)
    {
        nm_ranges.push_back((double)i);
        ev_ranges.push_back(ev_to_nm((double)i));
    }
    for (TimeStep ts : timesteps)
    {
        ts.GenerateSpectralData(ev_ranges);
        // Iterate through all timesteps, generating spectral data for each transition
    }
}

void OverallDataStructure::WriteOutputFile()
{
    // Number of lines per timestep, based on the size of the ev_ranges array of energy values.
    int n_lines = ev_ranges.size();

    // need column for ev, nm, then each transition
    std::stringstream buffer;
    buffer.str("");
    buffer << "Energy(eV), Energy(nm)";
    for (std::string t_name : known_transition_names)
    {
        buffer << ", " << t_name;
    }
    buffer << std::endl;

    // for each timestep, we need the full set of columns iterated through.
    for (TimeStep ts: timesteps)
    {
        // each timestep needs the energy values in eV, nM and then the intensity of each transition
        for (int i = 0; i < n_lines; i++)
        {
            buffer << std::fixed << std::setprecision(5) << ev_ranges[i] << ", ";
            buffer << std::fixed << std::setprecision(5) << nm_ranges[i];
            for (std::string t_name : known_transition_names)
            {
                buffer << ", ";
                buffer << std::fixed << std::setprecision(5) << ts.spectral_data[t_name][i];
            }
            buffer << std::endl;
        }
    }

    std::ofstream ofile(output_csv_filename,std::ios::out);
    ofile << buffer.str();
    ofile.close();
}

void OverallDataStructure::AddTimestep()
{
    timesteps.push_back(TimeStep());
}

void OverallDataStructure::AddExcitation(std::string transition_name, double ene, double osc_str)
{
    timesteps.back().AddExcitation(transition_name, ene, osc_str);
    if (std::find(known_transition_names.begin(), known_transition_names.end(), transition_name) == known_transition_names.end())
    {
        known_transition_names.push_back(transition_name);
    }
}

void OverallDataStructure::PlotSpectra()
{
    // include spans for visible, IR, UV, etc. on the main plot
    // Set x limits to the min_nm and max_nm values in this class.
}

void OverallDataStructure::reset()
{
    min_nm = 200;
    max_nm = 800;
    known_transition_names.clear();
    // std::vector<TimeStep> timesteps;
    timesteps.clear();
    std::stringstream buffer;
    int file_count = 0;
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