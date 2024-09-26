#include "classes.h"
#include "utilities.h"
#include "parsers.h"

bool DEBUGGING = false;

int main(int argc, char** argv)
{
    /* 
        This program parses TeraChem output files into a CSV to allow 
        for more standardized processing/plotting via python scripts.

        Accepted command line arguments are "-debug" and filenames.
        Filenames given with wildcards (*) should be expanded by the shell itself,
        and each filename given is checked for existence before being included in the list to process.
        Missing files will be listed to the end user during initialization.
    */
    std::stringstream buffer;
    // Obtain filenames/wildcards/list from arguments, validate as we go!
    std::vector <std::string> file_list = get_file_list(argc,argv);
    
    // Iterate through file list:    
    for (std::string it : file_list)
    {
        buffer.str("");
        buffer << "Parsing file: " << it << std::endl;
        debug(buffer.str());
        JobSettings current(it);
        if (current.runtype == "OPT")
        {
            parse_opt(current);
        }
        else if (current.runtype == "BOMD")
        {
            parse_bomd(current);
        }
        else if (current.runtype == "SPE")
        {
            parse_spe(current);
        }
    }

    // Notes:  If multiple files, output results of each file to a csv of the same filename?

    return 0;
}