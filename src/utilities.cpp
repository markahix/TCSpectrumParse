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
        if (std::string(argv[i]) == "-combine")
        {
            COMBINE_FILES = true;
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

double ev_to_nm(double ev)
{
    return 1239.8/ev;
}

std::vector<double> gauss(double a, double x_naught)
{
    /*
        Function takes an energy (eV) and coefficient (unitless) and converts to a gaussian-smoothed curve from 200nm to 800nm.
    */
    /*
        def gauss(x, a, x0, sigma):
            return a * np.exp(-(x - x0)**2 / (2 * sigma**2))
    */
    std::vector<double> x_array = {};
    std::vector<double> results = {};

    // generate 200nm to 800nm array, increment by 1nm
    for (int i=200; i <=800; i++)
    {
        x_array.push_back(ev_to_nm((double)i));
    }
    
    // populate results array with gaussian values at each nm.
    for (double x : x_array)
    {   // sigma value = 0.05 --> condense and simplify formula...
        results.push_back( a * exp( -200 * pow(x - x_naught, 2) ) );
    }
    return results;
}

std::vector<double> gauss(double a, double x_naught, std::vector<double> ev_ranges)
{
    std::vector<double> results = {};
    // populate results array with gaussian values at each nm.
    for (double x : ev_ranges)
    {   // sigma value = 0.05 --> condense and simplify formula...
        results.push_back( a * exp( -200 * pow(x - x_naught, 2) ) );
    }
    return results;
}