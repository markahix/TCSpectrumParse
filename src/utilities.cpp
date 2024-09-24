#include "utilities.h"

void debug(std::string printstring)
{
    if (DEBUGGING)
    {
        std::cout << printstring << std::endl;
    }
}

std::vector<std::string> get_file_list(int argc, char** argv)
{
    std::vector <std::string> file_list;
    for (int i=1; i < argc; i++)
    {
        if (std::string(argv[i]) == "-debug")
        {
            DEBUGGING = true;
            continue;
        }
        if (std::experimental::filesystem::exists(argv[i]) )
        {
            std::cout << argv[i] << " found!  Adding to processing queue." << std::endl;
            file_list.push_back(argv[i]);
        }
        else
        {
            std::cout << argv[i] << " not found." << std::endl;
        }
    }
    return file_list;
}