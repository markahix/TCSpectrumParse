#include "classes.h"
#include "utilities.h"
#include "parsers.h"

bool DEBUGGING = false;
bool COMBINE_FILES = false;

// int main(int argc, char** argv)
// {
    // /* 
    //     This program parses TeraChem output files into a CSV to allow 
    //     for more standardized processing/plotting via python scripts.

    //     Accepted command line arguments are "-debug" and filenames.
    //     Filenames given with wildcards (*) should be expanded by the shell itself,
    //     and each filename given is checked for existence before being included in the list to process.
    //     Missing files will be listed to the end user during initialization.
    // */
    // std::stringstream buffer;
    // // Obtain filenames/wildcards/list from arguments, validate as we go!
    // std::vector <std::string> file_list = get_file_list(argc, argv);
    
    // // Iterate through file list:    
    // for (std::string it : file_list)
    // {
    //     buffer.str("");
    //     buffer << "Parsing file: " << it << std::endl;
    //     debug(buffer.str());
    //     JobSettings current(it);
    //     if (current.runtype == "OPT")
    //     {
    //         parse_opt(current);
    //     }
    //     else if (current.runtype == "BOMD")
    //     {
    //         if (current.calctype == "SA-CASSCF")
    //         {
    //             BOMD_SACASSCF_to_CSV(it);
    //         }
    //         else
    //         {
    //             parse_bomd(current);
    //         }
            
    //     }
    //     else if (current.runtype == "SPE")
    //     {
    //         parse_spe(current);
    //     }
    // }

    // // Notes:  If multiple files, output results of each file to a csv of the same filename?
    
    // if ((COMBINE_FILES) && (file_list.size() > 1))
    // {
    //     // make multi-line plot
    //     plot_multi_csv(file_list);
    // }

    // return 0;
// }

int main(int argc, char** argv)
{
    // Parse command line for file list and keyword flags like '-debug' and '-combine'
    std::vector <std::string> file_list = get_file_list(argc, argv);

    // Initialize the OverallDataStructure
    OverallDataStructure ods;

    std::cout << "Built the OverallDataStructure" << std::endl;

    for (std::string file : file_list)
    {
        // Handle each file.
        ods.AddInputFile(file);
        std::cout << "Added input file: " << file << std::endl;
        // If we're not combining everything into a single file, save the current structure to its output file and run reset();
        if (!COMBINE_FILES)
        {
            ods.GenerateSpectralData();
            ods.WriteOutputFile();
            ods.reset();
        }
    }
    std::cout << "Finished Adding Input Files." << std::endl;
    ods.GenerateSpectralData();
    ods.WriteOutputFile();
    ods.reset();

    // All output files have been written
    return 0;
}