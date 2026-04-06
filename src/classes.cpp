#include "classes.h"
#include "utilities.h"

void ParseSPE(InputFile &input)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(input.filename(),std::ios::in);
    while (getline(file, line))
    {
        block.push_back(line);
    }
    file.close();

    if (input.getcalctype() == "TDDFT")
    {
        input.excitations = ParseTDDFTBlock(block);
    }
    if (input.getcalctype() == "SA-CASSCF")
    {
        input.excitations = ParseSACASSCFBlock(block);
    }
    input.Write_CSV();
}

void ParseOPT(InputFile &input)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(input.filename(),std::ios::in);
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

    if (input.getcalctype() == "TDDFT")
    {
        input.excitations = ParseTDDFTBlock(block);
    }
    if (input.getcalctype() == "SA-CASSCF")
    {
        input.excitations = ParseSACASSCFBlock(block);
    }
    input.Write_CSV();
}

void ParseBOMD(InputFile &input)
{
    std::string line;
    std::stringstream dummy;
    std::vector<std::string> block;
    std::ifstream file(input.filename(),std::ios::in);
    while (getline(file,line))
    {
        block.push_back(line);
        if (line.find("MD STEP") != std::string::npos)
        {
            if (input.getcalctype() == "TDDFT")
            {
                input.excitations = ParseTDDFTBlock(block);
            }
            if (input.getcalctype() == "SA-CASSCF")
            {
                input.excitations = ParseSACASSCFBlock(block);
            }
            input.Write_CSV();
            block.clear();
        }
    }
    file.close();
    if (input.getcalctype() == "TDDFT")
    {
        input.excitations = ParseTDDFTBlock(block);
    }
    if (input.getcalctype() == "SA-CASSCF")
    {
        input.excitations = ParseSACASSCFBlock(block);
    }
    input.Write_CSV();
}

std::vector<Excitation> ParseTDDFTBlock(std::vector<std::string> block)
{
    std::vector<Excitation> excitations = {};
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
            Excitation newexcite = {stof(excite), stof(osci),"S0->S1"};
            excitations.push_back(newexcite);
            // AddExcitation("TDDFT", stof(excite), stof(osci));
        }
    }
    return excitations;
}

std::vector<Excitation> ParseSACASSCFBlock(std::vector<std::string> block)
{
    std::vector<Excitation> excitations = {};
    std::map<std::string, double> state_energies = {};
    std::map<std::string, int> mult_count = {{"singlet",-1},{"doublet",-1},{"triplet",-1},{"quartet",-1}}; // Start at -1 so that the first encountered multiplicity state is S0, D0, T0, etc.
    std::map<std::string,std::string> multiplicity_prefixes = {{"singlet","S"},{"doublet","D"},{"triplet","T"},{"quartet","Q"}};
    std::vector<std::string> state_names = {};
    std::string current_mult = "S";
    std::map<std::string, double> excitation_energies = {};

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
                Excitation new_exc = {curr_energy, osc_str, t_name.str()};
                excitations.push_back(new_exc);
                i++;
                line = block[i];
            }
        }
    }
    return excitations;

}

InputFile::InputFile(std::string filename)
{
    // Make sure output file is complete...
    // std::ifstream file(filename,std::ios::in);
    // std::string line;
    bool is_valid=true;
    // while(getline(file,line))
    // {
    //     if (line.find("Job finished") != std::string::npos)
    //     {
    //         is_valid = true;
    //     }
    // }
    // file.close();
    if (is_valid)
    {
        // Generate initial information from filename.
        inputfile = filename;
        outputcsv = filename.substr(0,filename.find_last_of('.'))+".csv";
        if (std::experimental::filesystem::exists(outputcsv)) // Remove output csv for previous run.
        {
            std::experimental::filesystem::remove(outputcsv);
        }
        runtype = GetRunType(filename);
        calctype = GetCalcType(filename);

        // Report input file information
        std::cout << "Input Filename:   " << inputfile << std::endl;
        std::cout << "Output Filename:  " << outputcsv << std::endl;
        std::cout << "Run type:         " << runtype << std::endl;
        std::cout << "Calculation Type: " << calctype << std::endl << std::endl;
    }
    else
    {
        inputfile = filename;
        runtype = "";
        calctype = "";
        std::cout << "Input File " << inputfile << " was found to be invalid." << std::endl;
    }
}

InputFile::~InputFile()
{

}

std::string InputFile::filename()
{
    return inputfile;
}

std::string InputFile::csvname()
{
    return outputcsv;
}

std::string InputFile::getruntype()
{
    return runtype;
}

std::string InputFile::getcalctype()
{
    return calctype;
}

void InputFile::Write_CSV()
{
    if (!std::experimental::filesystem::exists(outputcsv))
    {
        std::ofstream csv(outputcsv, std::ios::out);
        csv << "Energy(eV), Energy(nm)";
        for (Excitation ex: excitations)
        {
            csv << ", " << std::setw(8) << ex.transition_name;
        }
        csv << std::endl;
        csv.close();
    }
    std::ofstream csv(outputcsv, std::ios::app);
    if (!csv.is_open())
    {
        std::cout << "Unable to open " << outputcsv << " for writing.  Terminating run." << std::endl;
        exit(1);
    }
    
    if (excitations.size() > 0)
    {
        for (int i=1000; i > 199; i--)
        {
            double ev = 1239.8/((double)i);
            csv << std::fixed << std::setprecision(4) << std::setw(10) << ev << ", ";
            csv << std::fixed << std::setprecision(4) << std::setw(10) << (double)i;
            for (Excitation ex: excitations)
            {
                double intensity = gauss_at_energy(ex.oscillator, ex.energy, ev);
                csv << ", " << std::fixed << std::setprecision(4) << std::setw(8) << intensity;
            } 
            csv << std::endl;
        }
    }
    csv.close();

    // Clear the Excitations
    excitations.clear();

}

void GeneratePythonPlottingScript(std::vector<std::string> csv_names)
{
    std::string python_script=R"PYTHON(#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import sys
from glob import glob as G
import pandas as pd

def csv_to_png(csv):
    figfilename = csv.replace(".csv",".png")
    consolidated_csv = csv.replace(".csv","_combined_data.csv")
    fig = plt.figure(figsize=[8,5], dpi=300)
    ax = fig.add_subplot(1,1,1)
    for chunk in pd.read_csv(csv, delimiter=',', chunksize=1):
        t_cols = [x.strip() for x in chunk.columns if "S0->" in x]
        break
    combined_data = {x:np.zeros(801) for x in t_cols}
    colors = [cm.get_cmap("viridis")(x) for x in np.linspace(0,1,len(t_cols))]
    n_spectra = 0
    for df in pd.read_csv(csv,delimiter=',',chunksize=801,dtype=float):
        df = df.rename(columns={x:x.strip() for x in df.columns})
        x_values = df["Energy(nm)"]
        for i,col in enumerate(t_cols):
            ax.plot(x_values, df[col], color=colors[i],lw=0.5,alpha=0.05)
            if col not in [x for x in combined_data.keys()]:
                combined_data[col] = df[col].values
            else:
                combined_data[col] += df[col].values
        n_spectra+=1
    final_combined_data = np.zeros(801)
    i=0
    for key, val in combined_data.items():
        ax.plot(x_values, val/n_spectra, color=colors[i],lw=1.0,alpha=0.75,label = key)
        i+=1
        final_combined_data += val/n_spectra
    ax.plot(x_values,final_combined_data,color="k", label= "Complete Spectrum",lw=1.0)
    ax.set_xlim(200,1000)
    ax.set_xlabel("Energy (nm)")
    ax.set_ylabel("Intensity")
    ax.legend()
    fig.savefig(figfilename,dpi=300,facecolor="white",bbox_inches="tight")
    np.savetxt(consolidated_csv,np.asarray([x_values,final_combined_data]).T,fmt="%.04f",header="Energy(nm), Intensity",delimiter=', ')

if __name__ == "__main__":
    csv_files=[]
    for arg in sys.argv[1:]:
        print(arg)
        csv_files.append(arg)
    for csv in csv_files:
        csv_to_png(csv)
)PYTHON";
    std::ofstream python("tmp.py", std::ios::out);
    python << python_script;
    python.close();
    std::stringstream buffer;
    buffer.str("");
    buffer << "python tmp.py ";
    for (std::string csv : csv_names)
    {
        buffer << csv << " ";
    }
    buffer << "; rm tmp.py"; 
    silent_shell(buffer.str().c_str());
}