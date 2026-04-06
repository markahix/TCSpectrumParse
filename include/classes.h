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

struct Excitation
{
    double energy;
    double oscillator;
    std::string transition_name;
};

class InputFile
{
    private:
        std::string inputfile;
        std::string runtype;
        std::string calctype;
        std::string outputcsv;
        std::vector<double> energies_ev;


    public:
        std::string filename();
        std::string csvname();
        std::string getruntype();
        std::string getcalctype();
        InputFile(std::string filename);
        ~InputFile();
        std::vector<Excitation> excitations;
        void Write_CSV();
};

std::vector<Excitation> ParseTDDFTBlock(std::vector<std::string> block);
std::vector<Excitation> ParseSACASSCFBlock(std::vector<std::string> block);

void ParseSPE(InputFile &input);
void ParseOPT(InputFile &input);
void ParseBOMD(InputFile &input);

void GeneratePythonPlottingScript(std::vector<std::string> csv_names);


#endif