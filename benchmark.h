/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __BENCHMARK__H__
#define __BENCHMARK__H__


#include "globals.h"
#include "api.h"

#include <vector>
#include <string>


// ~ Function that estimates a (mean, std) pair from sample
std::pair<ui64, ui64> Statistics(const std::vector<ui64>& sample);


// ~ Class describing the workload pattern
struct TPattern {
    // ~ Flag showing that the memory access is _consecutive_ (instead of _random_)
    bool IsConsecutive = false;
    // ~ Flag showing that the memory access operation is _read_ (instead of _write_)
    bool IsRead = false;
};


// ~ Class storing factor levels
// Characterizes a point in the factor space
class TFactorLevels {
public:
    void SetLevel(const std::string& factor, ui64 level);

    ui64 GetLevel(const std::string& factor) const;

public:
    // ~ Average size of the memory access request (in bytes)
    ui64 RequestSize = 64_KB;
    // ~ Depth of the maintained queue
    // Queue is maintained through readV and writeV operations.
    ui64 QueueDepth = 8;
    // ~ Flag to skip cache
    ui64 DirectIO = 0;
};


// ~ Class storing warmup parameters
struct TWarmupParams {
    // ~ Coef used in warmup completion criterion
    // The criterion: std <= ThresholdCoef * mean.
    double ThresholdCoef;
    // ~ Maximum warmup duration (in microseconds)
    ui64 MaxDuration;
    // ~ Size of the latencies sample used in statistics calculation
    ui32 SampleSize;
};


// ~ Class storing environment parameters
struct TEnvironmentParams {
    std::string Filepath; // ~ Path to the file to be tested
    ui64 Filesize; // ~ Size of the file to be tested (in bytes)
    bool Unlink = true; // ~ Flag showing that the file should be removed if it exists
    std::string PreparationScript = ""; // ~ Script called at the beginning of preparation
};


// ~ Main class
// Intended for benchmarking a single point in the factor space
class TBenchmark {
public:
    TBenchmark(TPattern pattern, const TFactorLevels& factorLevels,
               const TWarmupParams& warmup, const TEnvironmentParams& environment,
               ui64 testDuration, ui32 batchSize, IAPIFactory* factory);

    // ~ Main benchmarking method
    // Performs a single benchmark.
    // Returns a vector of latencies.
    std::vector<ui64> Benchmark();

private:
    // ~ Method that prepares the environment
    ui32 PrepareEnvironment() const;

// ~ Benchmark parameters stored for multiple use
private:
    TPattern Pattern;
    TFactorLevels FactorLevels;
    TWarmupParams Warmup;
    TEnvironmentParams Environment;
    // ~ Duration of a benchmark run (in microseconds)
    ui64 TestDuration;
    // ~ Amount of operations in a group for which the latency is measured
    ui32 BatchSize;
    // ~ Factory for producting IAPI* objects
    // | Stores the objects created.
    IAPIFactory* Factory;
    // ~ Min amount of iterations to be performed during testing (excluding warmup)
    // Parameter is set in the first benchmark run.
    ui64 MinIterations = 0;
};






#endif
