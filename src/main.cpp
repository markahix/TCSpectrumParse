#include "classes.h"
#include "utilities.h"

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
   
    // Obtain filenames/wildcards/list from arguments, validate as we go!
    std::vector <std::string> file_list = get_file_list(argc,argv);

    // Iterate through file list:    

    // 1. Verify the file is a TeraChem output file.

    // 2. Identify what kind of output it was (optimization, spe, MD, etc.)

    // 3. Identify what job settings were used (state averaged, tddft, casscf, etc.)

    // 4. Direct file to appropriate parsing function.

    // 5. Output to CSV.

    // Notes:  If multiple files, output results of each file to a csv of the same filename?

    return 0;
}