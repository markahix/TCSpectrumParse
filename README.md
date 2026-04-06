# TCSpectrumParse

TeraChem is a powerful electronic structure program which can perform a wide array of QM and QM/MM calculations.  The output files from TeraChem are robust and contain tons of useful information about the electronic structure of a given system.

This program is designed to rapidly parse these outputs into easy-to-use CSV-formatted data tables which may be plotted into UV/Vis or IR spectra with conventional graphing software, including Python.

Currently, `tcparse` can identify the following runtypes:

- Geometry Optimization
- Single Point Energy
- QM/MM or Born-Oppenheimer Molecular Dynamics

It can also differentiate between TD-DFT and State-averaged CASSCF calculations.  Furthermore, when parsing SA-CASSCF outputs, it identifies electronic transitions from their respective spin multiplicities (singlet, triplet, etc.) and names each column in the dataset appropriately.  While the individual energy levels in the TeraChem output are listed numerically, the transitions are listed numerically in the subset of that multiplicity.  As a result, energy levels 1-8 may have 4 singlets and 4 triplets, however the triplet transition listed as 1->3 may actually correspond to energy levels 2 and 7 (as an example).

### To Do

- Incorporate [TCMDIR](https://github.com/markahix/TCMDIR) functionality into `tcparse` when encountering MD outputs.  This should streamline the overall parsing process considerably.