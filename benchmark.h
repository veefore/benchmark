/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __BENCHMARK__H__
#define __BENCHMARK__H__


#include "globals.h"
#include "api.h"

#include <vector>
#include <string>


// ~ Class describing the workload pattern
struct TPattern {
    // ~ Flag showing that the memory access is _consecutive_ (instead of _random_)
    bool IsConsecutive;
    // ~ Flag showing that the memory access operation is _read_ (instead of _write_)
    bool IsRead;
};


// ~ Class storing the factor levels
struct TFactors {
    // ~ Average size of the memory access request (in bytes)
    ui64 RequestSize;
    // ~ Depth of the maintained queue
    // Queue is maintained through readV and writeV operations.
    ui64 QueueDepth;
};


// ~ Class storing warmup parameters
struct TWarmupParams {
    // ~ Coef used in warmup stop criterion
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
    bool Unlink = true; // ~ Flag showing that the file should be removed if it exists
    ui64 Filesize; // ~ Size of the file to be tested (in bytes)
};


// ~ Main class
class TBenchmark {
public:
    TBenchmark(const TPattern& pattern, const TFactors& factors,
               const TWarmupParams& warmup, const TEnvironmentParams& environment,
               ui64 testDuration, ui32 batchSize, IAPIFactory* factory);

    // ~ Main benchmarking method
    // Can be called multiple times.
    std::vector<ui64> Benchmark();

private:
    // ~ Method that prepares the environment
    ui32 PrepareEnvironment();

// ~ Benchmark parameters stored for multiple use
private:
    TPattern Pattern;
    TFactors Factors;
    TWarmupParams Warmup;
    TEnvironmentParams Environment;
    ui64 TestDuration; // ~ Duration of a benchmark run (in microseconds)
    ui32 BatchSize; // ~ Amount of operations in a group for which the latency is measured
    IAPIFactory* Factory; // ~ Factory for producting IAPI* objects | Stores the objects created.
};


std::pair<ui64, ui64> Statistics(const std::vector<ui64>& sample);


#endif
