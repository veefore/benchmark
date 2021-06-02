/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __MAIN__CPP__
#define __MAIN__CPP__


//#include "test.h"
#include "io.h"

//using namespace std;

int main() {
    //RunTests();
    auto experimenter = ReadExperiment();
    auto results = experimenter.Experiment();
    PrintExperimentResults(results,
                           experimenter.GetPattern(),
                           experimenter.GetFactorLevels(),
                           experimenter.GetVaryingFactors());
    return 0;
}


#endif
