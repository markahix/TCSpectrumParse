#ifndef PARSERS_H
#define PARSERS_H

#include "classes.h"
#include "utilities.h"
#include "spectra.h"

void parse_bomd(JobSettings current);
void parse_spe(JobSettings current);
void parse_opt(JobSettings current);
void plot_multi_csv(std::vector<std::string> file_list);
void BOMD_SACASSCF_to_CSV(std::string filename);

#endif