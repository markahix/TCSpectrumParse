#include "utilities.h"
#include "parsers.h"
#include "spectra.h"

/*
Need BOMD/SA-CASSCF parser with multiple transitions accounted for.
Energies based on min- and max-values identified (with minimum range of visible spectrum)

For each frame of MD in the output file, the CSV should have Energies vs. Transition as a matrix, with the intensities at each pair, then repeat the matrix again for each frame.

This will mean the python script will also need to be rebuilt.

*/

void BOMD_SACASSCF_to_CSV(std::string filename)
{
    std::string csvfile = filename.substr(0, filename.find_last_of('.')) + ".csv";
    if (std::experimental::filesystem::exists(csvfile))  // if the CSV doesn't exist, we should create it with column headers
    {
        std::cout << "Removing " << csvfile << std::endl;
        std::experimental::filesystem::remove(csvfile);
    }
    std::ifstream ifile(filename, std::ios::in);
    std::string line;
    
    std::map<std::string, double> state_energies         = {};
    std::vector<std::string> state_names                 = {};
    std::vector<std::string> transition_names            = {};
    std::map<std::string, double> excitation_energies    = {};
    std::map<std::string, double> oscillator_strengths   = {};
    std::map<std::string,std::vector<double>> MD_spectra = {};
    bool get_state_names = true; // only need this to happen on the first round of MD parsing.
    bool get_trans_names = true; // only need this to happen on the first round of MD parsing.

    while(getline(ifile,line)) // main reading loop.
    {
        // As we are reading the file, the first thing we'll encounter is "State Averaged Energy", followed by N singlet states (or possibly triplets?)
        //"Root   Mult.   Total Energy (a.u.)   Ex. Energy (a.u.)     Ex. Energy (eV)"
        if (line.find("Root   Mult.   Total Energy (a.u.)   Ex. Energy (a.u.)     Ex. Energy (eV)") != std::string::npos)
        {
            // skip the line of dashes.
            getline(ifile,line);
            // read the energies of each line
            while(getline(ifile,line))
            {
                if (line == "")
                {
                    break;
                }
                std::string state;
                std::string mult;
                double energy;
                std::stringstream buffer;

                // get state number, multiplicity, and energy value
                buffer.str(line);
                buffer >> state >> mult >> energy;
                buffer.str("");
                state_energies[state] = energy;
                if (get_state_names)
                {
                    state_names.push_back(state);
                }
            }
            // we hit that empty line, so we're out of the energies block.  Now we need to calculate the excitation energies between each of the states.
            for (int i = 0; i < state_names.size()-1 ; i++)
            {
                for (int j=i+1; j < state_names.size(); j++)
                {
                    std::string transition_name=state_names[i]+"->"+state_names[j];
                    double tmp_exc = (state_energies[state_names[j]] - state_energies[state_names[i]]) * 27.211324570273;
                    excitation_energies[transition_name] = tmp_exc;
                    if (get_trans_names)
                    {
                        transition_names.push_back(transition_name);
                    }
                }
            }
            get_state_names = false;
            get_trans_names = false;
        }

        if (line.find("Transition      Tx        Ty        Tz       |T|    Osc. (a.u.)") != std::string::npos)
        {
            // skip the line of dashes.
            getline(ifile,line);
            while (getline(ifile,line))
            {
                // Obtaining oscillator strengths of the individual transitions.
                if (line == "")
                {
                    break;
                }
                std::string from_state;
                std::string to_state;
                std::string transition_name;
                std::string junk;
                double osc_str;
                std::stringstream buffer;
                buffer.str(line);
                buffer >> from_state >> junk >> to_state >> junk >> junk >> junk >> junk >> osc_str;
                buffer.str("");
                transition_name = from_state + "->" + to_state;
                oscillator_strengths[transition_name] = osc_str;
            }

            // now we've obtained all the oscillator strengths for the transitions identified and exited that miniloop.
            // We can now calculate the spectra for each transition at the current frame
            for (std::string tr_name : transition_names)
            {
                double exc = excitation_energies[tr_name];
                double osc = oscillator_strengths[tr_name];
                MD_spectra[tr_name] = gauss(osc, exc);
            }

            // We have all the MD spectra, we can dump it into the CSV file
            std::stringstream file_data;
            file_data.str("");
            if (!std::experimental::filesystem::exists(csvfile))  // if the CSV doesn't exist, we should create it with column headers
            {
                file_data << "Energies,";
                for (std::string tr_name : transition_names)
                {
                    file_data << tr_name << ",";
                }
                file_data << std::endl;
            }
            for (int i=0; i < MD_spectra[transition_names[0]].size(); i++)
            {
                file_data << ev_to_nm((double)(i+200)) << ",";
                for (std::string tr_name : transition_names)
                {
                    file_data << std::fixed << std::setprecision(4) << MD_spectra[tr_name][i] << ",";
                }
                file_data << std::endl;
            }
            std::ofstream ofile(csvfile,std::ios::app);
            ofile << file_data.str();
            ofile.close();
        }
    }
    ifile.close();
    
    std::ofstream python(".plotthis.py", std::ios::out);
    python << R""""(
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm

def ProcessMultiCSVToCumulative(csvfile):
    all_data = np.genfromtxt(csvfile,names=True,dtype=float,delimiter=',')
    n_rows_per_frame = len(set(all_data["Energies"]))

    fig = plt.figure(figsize=[8,5],dpi=300)
    ax = fig.add_subplot(1,1,1)
    column_names = [x for x in all_data.dtype.names][1:-1]
    colors = [cm.get_cmap("viridis")(x) for x in np.linspace(0,1,len(column_names))]

    cumulatives = {}
    nframes = 0
    for i in range(0,len(all_data["Energies"]),n_rows_per_frame):
        s = i
        e = i+n_rows_per_frame
        x_values = [1239.8/x for x in all_data["Energies"][s:e]]
        for j,colname in enumerate(column_names):
            if i == 0:
                ax.plot(x_values,all_data[colname][s:e],color=colors[j],lw=0.1, alpha=0.1)
                cumulatives[colname] = all_data[colname][s:e]
            else:
                ax.plot(x_values,all_data[colname][s:e],color=colors[j],lw=0.1, alpha=0.1)
                cumulatives[colname] += all_data[colname][s:e]
        nframes+=1

    for i,col in enumerate(column_names):
        ax.plot(x_values, cumulatives[col]/nframes, color = colors[i], label = "->".join([x for x in col]),lw=1.0,alpha=0.5)
        if i == 0:
            single_spectrum = cumulatives[col]/nframes
        else:
            single_spectrum += cumulatives[col]/nframes
    ax.plot(x_values,single_spectrum,color="k",lw=1.0,alpha=1.0)
    ax.legend()
    ax.set_xlabel("Energy (nm)")
    ax.set_xlim(200,800)
    ax.set_ylim(0,None)
    ax.set_ylabel("Intensity (a.u.)")
    fig.savefig(csvfile.replace(".csv",".png"),facecolor="white",dpi=300)
    xydata =[(x,y) for x,y in zip(x_values,single_spectrum)]
    np.savetxt(csvfile.replace(".csv","_cumulative_data.csv"),xydata,delimiter=',',fmt="%.04f")
)"""";
    python << "ProcessMultiCSVToCumulative('" << csvfile << "')" << std::endl;
    silent_shell("python .plotthis.py && rm .plotthis.py");
}
    