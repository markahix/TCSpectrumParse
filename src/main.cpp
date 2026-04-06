#include "classes.h"
#include "utilities.h"

bool DEBUGGING = false;
bool COMBINE_FILES = false;

int main(int argc, char** argv)
{
    // Parse command line for file list and keyword flags like '-debug' and '-combine'
    std::vector <std::string> file_list = get_file_list(argc, argv);
    std::vector<std::string> csv_names = {};

    for (std::string file : file_list)
    {
        // Handle each file.
        InputFile inp(file);
        if (inp.getruntype() == "")
        {
            continue;
        }
        csv_names.push_back(inp.csvname());
        if (inp.getruntype() == "SPE")
        {
            ParseSPE(inp);
        }
        if (inp.getruntype() == "OPT")
        {
            ParseOPT(inp);
        }
        if (inp.getruntype() == "BOMD")
        {
            ParseBOMD(inp);
        }
    }
    std::cout << "Finished Adding Input Files." << std::endl;
    std::string combined_csv_filename="";
    if (COMBINE_FILES)
    {
        csv_names.push_back(Combine_CSVs(csv_names));
    }
    
    // All output files have been written
    GeneratePythonPlottingScript(csv_names);

    return 0;
}