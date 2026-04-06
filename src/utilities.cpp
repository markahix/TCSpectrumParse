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
    if (std::experimental::filesystem::exists("tcparse.log"))
    {
        std::experimental::filesystem::remove("tcparse.log");
    }
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

double gauss_at_energy(double osc, double energy_peak, double energy_eval)
{
    return osc * exp( -200 * pow(energy_eval - energy_peak, 2) );
}

std::string GetRunType(std::string filename)
{
    std::stringstream buffer;
    std::ifstream file(filename, std::ios::in);
    std::string line;
    std::string calctype = "";
    while (getline(file,line))
    {
        buffer.str("");
        // IDENTIFY RUNTYPE
        if (line.find("RUNNING AB INITIO MOLECULAR DYNAMICS") != std::string::npos)
        {
            // need to extract many sets of values
            calctype = "BOMD";
        }
        if (line.find("RUNNING GEOMETRY OPTMIZATION") != std::string::npos)
        {
            // need to extract only one set of values
            calctype = "OPT";
        }
        if (line.find("SINGLE POINT ENERGY CALCULATIONS") != std::string::npos)
        {
            // need to extract only one set of values
            calctype = "SPE";
        }
    }
    file.close();
    return calctype;
}

std::string GetCalcType(std::string filename)
{
    std::stringstream buffer;
    std::ifstream file(filename, std::ios::in);
    std::string line;
    std::string runtype = "";
    while (getline(file,line))
    {
        buffer.str("");
         // IDENTIFY CALCTYPE
        if (line.find("CAS Parameters") != std::string::npos)
        {
            // I'm assuming CASSCF calculation will be the only one listing out CAS parameters...
            
            runtype = "SA-CASSCF";
        }
        if (line.find("DFT Functional requested") != std::string::npos)
        {
            runtype = "TDDFT";
        }
    }
    file.close();
    return runtype;
}

std::string Combine_CSVs(std::vector<std::string> csv_filenames)
{
    std::string base_dir = csv_filenames[0].substr(0,csv_filenames[0].find_last_of('/')+1);
    std::stringstream buffer;
    int n_files_found = 0;
    do 
    {
        buffer.str("");
        n_files_found++;
        buffer << base_dir << "Combined_Data_" << std::setw(4) << std::setfill('0') << n_files_found << ".csv";
    } while(std::experimental::filesystem::exists(buffer.str()));
    std::string combined_csv_filename = buffer.str();
    std::cout << "Combining CSVs into " << combined_csv_filename << std::endl;

    // Ensure header columns in all files match.
    std::string header_columns;
    for (int i = 0; i < csv_filenames.size(); i++)
    {
        std::ifstream file(csv_filenames[i], std::ios::in);
        std::string line;
        getline(file,line);
        file.close();
        if (i == 0)
        {
            header_columns = line;
        }
        else
        {
            if (line != header_columns)
            {
                std::cout << "Mismatch in header columns.  Terminating. " << std::endl;
                exit(1);
            }
        }
    }

    // Iterate through all CSVs, output their contents into the combined form.
    std::ofstream comb_csv(combined_csv_filename, std::ios::out);
    comb_csv << header_columns << std::endl;
    for (int i = 0; i < csv_filenames.size(); i++)
    {
        std::ifstream file(csv_filenames[i], std::ios::in);
        std::string line;
        getline(file,line); // skip header columns, we only need them once.
        while (getline(file,line))
        {
            comb_csv << line << std::endl;
        }
        file.close();
    }
    comb_csv.close();
    return combined_csv_filename;
}