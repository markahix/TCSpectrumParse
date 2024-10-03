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
        std::string dummy;
        
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
                buffer >> dummy >> dummy >> S0_energy;
                double excitation_energy = (S1_energy - S0_energy)* 27.211324570273; // 
                debug("Excitation energy is: " + std::to_string(excitation_energy));
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
                    debug("Oscillator value is: " + tmp);
                    Oscillator += stof(tmp);
                    count_transitions++;
                }
                oscillators.push_back(Oscillator/count_transitions);
            }
        }
    }

    else if (current.calctype == "TDDFT")
    {   
        for (int i=0;i < block.size(); i++)
        {
            std::string line=block[i];
            if (line.find("  Final Excited State Results:") != std::string::npos)
            {
            std::string dummy, excite, osci;
            std::stringstream buffer;
            buffer.str(block[i+4]);
            buffer >> dummy >> dummy >> excite >> osci;
            excitations.push_back(stof(excite));
            oscillators.push_back(stof(osci));
            debug("Excitation Energy: " + excite);
            debug("Oscillator: "+ osci);
            }
        }
    }

    debug("Length of excitations vector: " + std::to_string(excitations.size()));
    debug("Length of oscillators vector: " + std::to_string(oscillators.size()));
    // Now that we've parsed the given text block for excitations/oscillators, let's get an array of values!
    for (int i=0; i < excitations.size(); i++)
    {
        double exe = excitations[i];
        double osc = oscillators[i];
        std::vector<double> tmp_vec = gauss(osc, exe);
        debug("oscillators[i] = " + std::to_string(osc));
        debug("excitations[i] = " + std::to_string(exe));
        energies = add_vectors(energies, tmp_vec);
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
    debug("Generating MD_spectra vector.");
    for (int i = 800; i >= 200; i--)
    {
        std::vector<double> tmpvec;
        tmpvec.push_back(ev_to_nm((double)i));
        MD_spectra.push_back(tmpvec);
    }
    debug("Parsing BOMD outfile.");
    while (getline(file,line))
    {
        block.push_back(line);
        if (line.find("MD STEP") != std::string::npos) // each step of dynamics ends with the "MD STEP" number in all caps, and this flag appears nowhere else.)
        {   
            debug(line);
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
        outfile << std::fixed;
        std::vector<double> row = MD_spectra[i];
        for (double j : row)
        {
            outfile << j << " ";
        }
        outfile << std::endl;
    }
    outfile.close();

    std::ofstream python(".plotthis.py",std::ios::app);
    python << R""""(
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm

def csv_to_spectra_plot(csvfile,outfile):
    data = np.genfromtxt(csvfile,dtype=float)
    colors = [cm.get_cmap("inferno")(x) for x in np.linspace(0,1,len(data[:,0]))]
    eV = 1239.8/data[:,0]
    cumulative = np.zeros(len(data[:,0]))
    fig = plt.figure(figsize=[8,6],dpi=300,facecolor="white")
    ax = fig.add_subplot(1,1,1)
    for step in range(1,len(data)-1):
        y_data = data[:,step]
        cumulative+=y_data
        ax.plot(eV,y_data,color=colors[step],lw=0.5,alpha=0.5)

    cumulative /= (len(data[:,0])-1)
    ax.plot(eV,cumulative,color="k",lw=1.0)
    ax.set_xlabel("Energy (nm)")
    ax.set_xlim(min(eV),max(eV))
    ax.set_ylim(0,None)
    ax.set_ylabel("Intensity (a.u.)")
    fig.savefig(outfile,dpi=300,facecolor="white")
)"""";
    python << "csv_to_spectra_plot('" << outfilename << "','" << outfilename.substr(0,outfilename.size()-3) << "png')" << std::endl;

    silent_shell("python .plotthis.py; rm .plotthis.py");
}


void parse_spe(JobSettings current)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(current.inputfile);
    std::vector<std::vector<double>> MD_spectra;
    debug("Generating MD_spectra vector.");
    for (int i = 800; i >= 200; i--)
    {
        std::vector<double> tmpvec;
        tmpvec.push_back(ev_to_nm((double)i));
        MD_spectra.push_back(tmpvec);
    }
    debug("Parsing SPE outfile.");
    while (getline(file,line))
    {
        block.push_back(line);
    }

    std::vector<double> block_energies = parse_text_block(current,block);
    for (int i=0; i < block_energies.size(); i++)
    {
        MD_spectra[i].push_back(block_energies[i]); // each "column" of the MD_spectra 2D-array consists of the energies of a single 
    }
    // then clear the vector
    block.clear();

    // Now that we've parsed the whole file and built a 2D array, let's write the array to a CSV file of the same filename.
    std::string outfilename = current.inputfile.substr(0,current.inputfile.find('.')) + ".csv";
    std::ofstream outfile(outfilename,std::ios::app);
    for (int i=0; i < MD_spectra.size(); i++)
    {
        outfile << std::fixed;
        std::vector<double> row = MD_spectra[i];
        for (double j : row)
        {
            outfile << j << " ";
        }
        outfile << std::endl;
    }
    outfile.close();

    std::ofstream python(".plotthis.py",std::ios::app);
    python << R""""(
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm

def csv_to_spectra_plot(csvfile,outfile):
    data = np.genfromtxt(csvfile,dtype=float)
    eV = 1239.8/data[:,0]
    fig = plt.figure(figsize=[8,6],dpi=300,facecolor="white")
    ax = fig.add_subplot(1,1,1)
    y_data = data[:,1]
    ax.plot(eV,y_data,color="k",lw=1.0,alpha=1.0)

    ax.set_xlabel("Energy (nm)")
    ax.set_xlim(min(eV),max(eV))
    ax.set_ylim(0,None)
    ax.set_ylabel("Intensity (a.u.)")
    fig.savefig(outfile,dpi=300,facecolor="white")
)"""";
    python << "csv_to_spectra_plot('" << outfilename << "','" << outfilename.substr(0,outfilename.size()-3) << "png')" << std::endl;

    silent_shell("python .plotthis.py; rm .plotthis.py");
}


void parse_opt(JobSettings current)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(current.inputfile);
    std::vector<std::vector<double>> MD_spectra;
    debug("Generating MD_spectra vector.");
    for (int i = 800; i >= 200; i--)
    {
        std::vector<double> tmpvec;
        tmpvec.push_back(ev_to_nm((double)i));
        MD_spectra.push_back(tmpvec);
    }
    debug("Parsing OPT outfile.");
    while (getline(file,line))
    {
        if (line.find("Final Excited State Results:") != std::string::npos)
        {
            // We only want the final optimized results, not the entire process.
            block.clear();
        }
        block.push_back(line);
    }
    std::vector<double> block_energies = parse_text_block(current,block);
    for (int i=0; i < block_energies.size(); i++)
    {
        MD_spectra[i].push_back(block_energies[i]); // each "column" of the MD_spectra 2D-array consists of the energies of a single 
    }
    // then clear the vector
    block.clear();

    // Now that we've parsed the whole file and built a 2D array, let's write the array to a CSV file of the same filename.
    std::string outfilename = current.inputfile.substr(0,current.inputfile.find('.')) + ".csv";
    std::ofstream outfile(outfilename,std::ios::app);
    for (int i=0; i < MD_spectra.size(); i++)
    {
        outfile << std::fixed;
        std::vector<double> row = MD_spectra[i];
        for (double j : row)
        {
            outfile << j << " ";
        }
        outfile << std::endl;
    }
    outfile.close();

    std::ofstream python(".plotthis.py",std::ios::app);
    python << R""""(
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm

def csv_to_spectra_plot(csvfile,outfile):
    data = np.genfromtxt(csvfile,dtype=float)
    eV = 1239.8/data[:,0]
    fig = plt.figure(figsize=[8,6],dpi=300,facecolor="white")
    ax = fig.add_subplot(1,1,1)
    y_data = data[:,1]
    ax.plot(eV,y_data,color="k",lw=1.0,alpha=1.0)

    ax.set_xlabel("Energy (nm)")
    ax.set_xlim(min(eV),max(eV))
    ax.set_ylim(0,None)
    ax.set_ylabel("Intensity (a.u.)")
    fig.savefig(outfile,dpi=300,facecolor="white")
)"""";
    python << "csv_to_spectra_plot('" << outfilename << "','" << outfilename.substr(0,outfilename.size()-3) << "png')" << std::endl;

    silent_shell("python .plotthis.py; rm .plotthis.py");

}
