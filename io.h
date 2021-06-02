/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __IO__H__
#define __IO__H__


#include "experimenter.h"

#include <utility> // std::pair
#include <vector>
#include <string>


TExperimenter ReadExperiment();

TPattern ReadPattern();

std::pair<std::vector<TFactorLevels>, std::vector<std::string>> ReadFactorLevels();

std::pair<std::string, std::vector<ui64>> ReadFactor();

TWarmupParams ReadWarmupParams();

TEnvironmentParams ReadEnvironmentParams();


bool ReadBool(const std::string& message);

ui64 ReadUI64(const std::string& message);

ui32 ReadUI32(const std::string& message);


void PrintExperimentResults(const std::vector<std::pair<ui64, ui64>>& results,
                            TPattern pattern,
                            const std::vector<TFactorLevels>& factorLevels,
                            const std::vector<std::string>& varyingFactors);


void SetLevel(TFactorLevels& levels, const std::string& factor, ui64 level);

ui64 GetLevel(const TFactorLevels& levels, const std::string& factor, ui64 level);


#endif
