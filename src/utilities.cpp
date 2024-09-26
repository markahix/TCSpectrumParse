#include "utilities.h"

void debug(std::string printstring)
{
    if (DEBUGGING)
    {
        std::cout << printstring << std::endl;
    }
    else
    {
        std::ofstream logfile("tcparse.log",std::ios::app);
        logfile << printstring << std::endl;
        logfile.close();
    }
}

std::vector<std::string> get_file_list(int argc, char** argv)
{
    std::stringstream buffer;
    std::vector <std::string> file_list; 
    for (int i=1; i < argc; i++)
    {
        buffer.str("");
        if (std::string(argv[i]) == "-debug")
        {
            DEBUGGING = true;
            continue;
        }
        if (std::experimental::filesystem::exists(argv[i]) )
        {
            buffer << argv[i] << " found!  Adding to processing queue." << std::endl;
            debug(buffer.str());
            file_list.push_back(argv[i]);
        }
        else
        {
            buffer << argv[i] << " not found." << std::endl;
            debug(buffer.str());
        }
    }
    return file_list;
}
