#include "spectra.h"


double ev_to_nm(double eV)
{
    // converts between eV and nm.
    return 1239.8/eV;
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
    std::vector<double> x_array;
    std::vector<double> results;

    // generate 200nm to 800nm array, increment by 1nm
    for (int i=200; i <=800; i++)
    {
        x_array.push_back((double)i);
    }
    
    // populate results array with gaussian values at each nm.
    for (double x : x_array)
    {   // sigma value = 0.05 --> condense and simplify formula...
        results.push_back( a * exp( -200 * pow(x - x_naught, 2) ) );
    }
    return results;
}


