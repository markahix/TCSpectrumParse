#!/bin/bash
make
rm tcparse.log test_data/*.csv
# bin/tcparse test_data/mBaojin_972_test.out
bin/tcparse test_data/FusionRed_ES_spe.out
bin/tcparse test_data/Perylene_TDDFT_ES_spe.out
# bin/tcparse test_data/*.out
