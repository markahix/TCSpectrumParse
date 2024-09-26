#include "parsers.h"

std::vector<double> add_vectors(std::vector<double> vec_1, std::vector<double> vec_2)
{
    std::vector<double> results;
    for (int i=0; i<vec_1.size(); i++)
    {
        results.push_back(vec_1[i] + vec_2[i]);
    }
    return results;
}

// function to parse a block of text based on JobSettings.calctype (TDDFT vs. CASSCF), including state-averaging
std::vector<double> parse_text_block(JobSettings current, std::vector<std::string> block)
{
    // initialize the vector with zeros.
    std::vector<double> energies;
    for (int i = 200; i <= 800; i++)
    {
        energies.push_back(0.0);
    }

    std::vector<double> excitations;
    std::vector<double> oscillators;
    
    if (current.calctype == "SA-CASSCF")
    {
        double S0_energy,S1_energy;
        double Oscillator = 0.0;
        
        int start_osc,end_osc;

        for (int i=0; i < block.size(); i++)
        {
            std::string line = block[i];
            if (line.find("State Averaged Energy:") != std::string::npos)
            {
                // run through text block until we hit "State Averaged Energy"
                S1_energy = stof( line.substr( line.find(':') + 1, std::string::npos ) );
            }
            if (line.find("Root   Mult.   Total Energy (a.u.)   Ex. Energy (a.u.)     Ex. Energy (eV)     Ex. Energy (nm)") != std::string::npos)
            {
                std::stringstream buffer;
                buffer.str(block[i+2]);
                buffer >> S0_energy >> S0_energy >> S0_energy;
                double excitation_energy = (S1_energy - S0_energy) * 27.211324570273;
                excitations.push_back(excitation_energy);
            }
            if (line.find("Transition      Tx        Ty        Tz       |T|    Osc. (a.u.)") != std::string::npos)
            {
                start_osc = i+2;
            }
            if (line.find("state Mulliken charges:") != std::string::npos)
            {
                end_osc = i-1;
                int count_transitions = 0;
                for (int j = start_osc; j < end_osc; j++)
                {
                    std::string tmp;
                    std::stringstream buffer;
                    buffer.str(block[j]);
                    buffer >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp;
                    Oscillator += stof(tmp);
                    count_transitions++;
                }
                oscillators.push_back(Oscillator/count_transitions);
            }
        }
    }

    else if (current.calctype == "TDDFT")
    {   
        int start_line,end_line;
        for (int i=0; i < block.size(); i++)
        {
            std::string line=block[i];
            if (line.find("Root   Total Energy (a.u.)   Ex. Energy (eV)   Osc. (a.u.)") != std::string::npos)
            {
                start_line = i+2;
            }
            if (line.find("Energy calculation finished, energy:") != std::string::npos)
            {
                end_line = i-2;
            }
        }

        for (int i=start_line;i <end_line; i++)
        {
            std::string dummy, excite, osci;
            std::stringstream buffer;
            buffer.str(block[i]);
            buffer >> dummy >> dummy >> excite >> osci;
            excitations.push_back(ev_to_nm(stof(excite)));
            oscillators.push_back(stof(osci));
        }
    }

    // Now that we've parsed the given text block for excitations/oscillators, let's get an array of values!
    for (int i=0; i < energies.size(); i++)
    {
        energies = add_vectors(energies, gauss(oscillators[i], energies[i]) );
    }
    //send the final array of values back to the filescanner.
    return energies;
}
// should return a gaussian array of intensities  Probably should first pull the conversion function out of the original pythons script...

void parse_bomd(JobSettings current)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(current.inputfile);
    std::vector<std::vector<double>> MD_spectra;
    for (int i = 200; i <= 800; i++)
    {
        MD_spectra[i-200].push_back(i);
    }
    while (getline(file,line))
    {
        block.push_back(line);
        if (line.find("MD STEP") != std::string::npos) // each step of dynamics ends with the "MD STEP" number in all caps, and this flag appears nowhere else.)
        {   
            // push the vector to the block parser.
            std::vector<double> block_energies = parse_text_block(current,block);
            for (int i=0; i < block_energies.size(); i++)
            {
                MD_spectra[i].push_back(block_energies[i]); // each "column" of the MD_spectra 2D-array consists of the energies of a single 
            }
            // then clear the vector
            block.clear();
        }
    }
    file.close();
    // get the last chunk from MD.
    std::vector<double> block_energies = parse_text_block(current,block);
    for (int i=0; i < block_energies.size(); i++)
    {
        MD_spectra[i].push_back(block_energies[i]); // each "column" of the MD_spectra 2D-array consists of the energies of a single 
    }

    // Now that we've parsed the whole file and built a 2D array, let's write the array to a CSV file of the same filename.
    std::string outfilename = current.inputfile.substr(0,current.inputfile.find('.')) + ".csv";
    std::ofstream outfile(outfilename,std::ios::app);
    for (int i=0; i < MD_spectra.size(); i++)
    {
        std::vector<double> row = MD_spectra[i];
        for (double j : row)
        {
            outfile << j << ", ";
        }
        outfile << std::endl;
    }
    outfile.close();

}


void parse_spe(JobSettings current)
{
    
}


void parse_opt(JobSettings current)
{
    
}
