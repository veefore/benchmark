/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __IO__CPP__
#define __IO__CPP__


#include "io.h"

#include <iostream>
#include <stdexcept>
#include <unordered_map>


using std::cerr;
using std::cout;
using std::cin;


TExperimenter ReadExperiment() {
    cerr << "Please, enter experiment decription.\n";
    auto pattern = ReadPattern();
    auto [factorLevels, varyingFactors] = ReadFactorLevels();
    auto warmup = ReadWarmupParams();
    auto environment = ReadEnvironmentParams();
    ui64 testDuration = ReadUI64("Test duration (ms)") * 1000;
    ui32 batchSize = ReadUI32("Batch size");
    ui32 replays = ReadUI32("Replays");
    return TExperimenter(pattern, std::move(factorLevels), warmup, environment,
                         testDuration, batchSize, replays, varyingFactors);
}


TPattern ReadPattern() {
    cerr << "Reading workload pattern.\n";
    bool isConsecutive = ReadBool("Memory access is consecutive (random otherwise)");
    bool isRead = ReadBool("Memory access operation is read (write otherwise)");
    TPattern pattern;
    pattern.IsConsecutive = isConsecutive;
    pattern.IsRead = isRead;
    return pattern;
}


std::pair<std::vector<TFactorLevels>, std::vector<std::string>> ReadFactorLevels() {
    cerr << "Reading factors.\n";

    ui32 factors;
    cerr << "Number of factors: ";
    cin >> factors;
    if (cin.fail() || factors == 0)
        throw std::runtime_error("Error reading number of factors."
                            "Non-zero Integer was expected");
    
    std::unordered_map<std::string, std::vector<ui64>> factorsMap;
    std::vector<std::string> varyingFactors;
    for (ui32 i = 0; i < factors; i++) {
        auto [factor, levels] = ReadFactor();
        if (factorsMap.find(factor) != factorsMap.end())
            throw std::runtime_error("Duplicate specification of levels for factor " + factor +
                                     " is not supported.");
        factorsMap[factor] = levels;
        if (levels.size() > 1)
            varyingFactors.push_back(factor);
    }

    if (varyingFactors.size() > 2)
        throw std::runtime_error("There must be no more than 2 varying factors. " +
                                 std::to_string(varyingFactors.size()) + " is specified");

    // Generate all factor combinations as cartesian product
    std::vector<TFactorLevels> combinations(1);
    for (const auto [factor, levels] : factorsMap) {
        ui32 n = combinations.size();
        ui32 m = levels.size();
        combinations.resize(n * m);
        // Match each combination of already specified factors
        // with each level of current factor.
        for (ui32 i = 0; i < n; i++)
            for (ui32 j = 0; j < m; j++) {
                combinations[i + n * j] = combinations[i]; // excessive in case j == 0
                combinations[i + n * j].SetLevel(factor, levels[j]);
            }
    }

    return {combinations, varyingFactors};
}


std::pair<std::string, std::vector<ui64>> ReadFactor() {
    cerr << "Reading factor.\n";
    cerr << "Factors are:\n"
         << "\"RS\" for Request Size\n"
         << "Recommended range: [512B, 512MB] (Input value in Bytes!)\n"
         << "Default: 64KB\n"
         << "\"QD\" for Queue Depth\n"
         << "Recommended range: [1, 256]\n"
         << "Default: 8\n"
         << "\"DIO\" for Direct IO\n"
         << "Range: {0, 1}\n"
         << "Default: 0\n";
    std::string factor;
    cerr << "Factor name [\"RS\", \"QD\", \"DIO\"]: ";
    cin >> factor;
    if (factor != "RS" && factor != "QD" && factor != "DIO")
        throw std::runtime_error("Incorrect factor name: \"" + factor + "\""
                            "Supported values are: \"RS\", \"QD\", \"DIO\".");

    ui32 levels = ReadUI32("Factor levels count");
    if (levels == 0)
        throw std::runtime_error("You entered a factor with 0 levels");
    std::vector<ui64> factorLevels(levels);
    for (ui32 i = 0; i < levels; i++)
        factorLevels[i] = ReadUI64("Factor level #" + std::to_string(i + 1));

    return {factor, factorLevels};
}


TWarmupParams ReadWarmupParams() {
    cerr << "Reading warmup parameters.\n";
    TWarmupParams warmup;

    cerr << "Threshold coefficient (double): ";
    cin >> warmup.ThresholdCoef;
    if (cin.fail())
        throw std::runtime_error("Error reading double: invalid value passed");

    warmup.MaxDuration = ReadUI64("Max duration (ms)") * 1000;
    warmup.SampleSize = ReadUI32("Sample size (used in calculating mean and std)");

    return warmup;
}


TEnvironmentParams ReadEnvironmentParams() {
    cerr << "Reading environment parameters.\n";
    TEnvironmentParams environment;

    cerr << "Filepath: ";
    cin.get();
    getline(cin, environment.Filepath);

    environment.Filesize = ReadUI64("Filesize (MB)") * 1024 * 1024;
    environment.Unlink = ReadBool("Unlink file at filepath");

    cerr << "Preparation script: ";
    cin.get();
    getline(cin, environment.PreparationScript);

    return environment;
}


bool ReadBool(const std::string& message) {
    ui64 result;
    cerr << message << " [0 or 1]: ";
    cin >> result;
    if (cin.fail() || result > 1)
        throw std::runtime_error("Error reading bool: invalid value passed");
    return result;
}


ui64 ReadUI64(const std::string& message) {
    ui64 result;
    cerr << message << ": ";
    cin >> result;
    if (cin.fail())
        throw std::runtime_error("Error reading ui64. 64-bit integer was expected");
    return result;
}


ui32 ReadUI32(const std::string& message) {
    ui32 result;
    cerr << message << ": ";
    cin >> result;
    if (cin.fail())
        throw std::runtime_error("Error reading ui32: 32-bit integer was expected");
    return result;
}


void PrintExperimentResults(const std::vector<std::pair<ui64, ui64>>& result,
                            TPattern pattern,
                            const std::vector<TFactorLevels>& factorLevels,
                            const std::vector<std::string>& varyingFactors) {
    cout << pattern.IsConsecutive << "\n";
    cout << pattern.IsRead << "\n";
    cout << result.size() << "\n";
    ui32 factorsCnt = varyingFactors.size();
    cout << (factorsCnt <= 2 ? factorsCnt : 0) << "\n";
    for (ui64 i = 0; i < result.size(); i++) {
        auto [mean, std] = result[i];
        if (factorsCnt >= 1 && factorsCnt <= 2)
            cout << varyingFactors[0] << "\n"
                 << factorLevels[i].GetLevel(varyingFactors[0]) << "\n";
        if (factorsCnt == 2)
            cout << varyingFactors[1] << "\n"
                 << factorLevels[i].GetLevel(varyingFactors[1]) << "\n";
        cout << mean << "\n"
             << std << "\n";
    }
}



#endif
