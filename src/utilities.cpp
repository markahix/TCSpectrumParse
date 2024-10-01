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

void silent_shell(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
}
