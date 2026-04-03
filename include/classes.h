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
public:
    std::string inputfile;
    std::string runtype;
    std::string calctype;
    JobSettings(std::string filename);
    ~JobSettings();
};

struct Excitation
{
    double energy;
    double oscillator;
};

class TimeStep
{
    public:
        TimeStep();
        ~TimeStep();
        // each timestep has an map of transitions, with corresponding Excitation Data
        std::map<std::string, Excitation> excitations;
        std::map<std::string, std::vector<double>> spectral_data;
        void AddExcitation(std::string transition_name, double ene, double osc_str);
        void GenerateSpectralData(std::vector<double> ev_ranges);
};

class OverallDataStructure
{
    private:
        double min_nm;
        double max_nm;
        std::vector<double> ev_ranges;
        std::vector<double> nm_ranges;

        std::string output_csv_filename;
        std::string output_png_filename;
        std::vector<std::string> known_transition_names;
        void UpdateMinMaxEnergyRange();
        void ParseOPT(JobSettings current);
        void ParseSPE(JobSettings current);
        void ParseBOMD(JobSettings current);
        void ParseBlock(std::vector<std::string> block, std::string calctype);
        void ParseTDDFTBlock(std::vector<std::string> block);
        void ParseSACASSCFBlock(std::vector<std::string> block);
    public:
        OverallDataStructure();
        ~OverallDataStructure();
        // each datastructure is composed of a sequence of timesteps, frames, or individual files that get parsed into their respective excitations.
        std::vector<TimeStep> timesteps;
        
        // We can simply add any number of files to the mix, with each one being parsed into the datastructure as we go.
        void AddInputFile(std::string filename);
        void GenerateSpectralData();
        void WriteOutputFile();
        void AddTimestep();
        void AddExcitation(std::string transition_name, double ene, double osc_str);
        void PlotSpectra();
        void reset();
};


#endif