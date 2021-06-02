/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __EXPERIMENTER__H__
#define __EXPERIMENTER__H__


#include "benchmark.h"


void AddVectors(std::vector<ui64>& result, const std::vector<ui64>& toAdd);


// ~ Class for conducting multifactor experiments
// Intended for benchmarking multiple points in the factor space multiple times
class TExperimenter {
public:
    TExperimenter(TPattern pattern,
                  std::vector<TFactorLevels>&& factorLevels,
                  const TWarmupParams& warmup,
                  const TEnvironmentParams& environment,
                  ui64 testDuration,
                  ui32 batchSize,
                  ui32 replays,
                  const std::vector<std::string>& varyingFactors);

    // Performs an experiment and returns result in the same order
    // in which factorLevels were provided.
    std::vector<std::pair<ui64, ui64>> Experiment() const;

    TPattern GetPattern() const;

    const std::vector<TFactorLevels>& GetFactorLevels() const;

    const std::vector<std::string>& GetVaryingFactors() const;

private:
    std::vector<ui32> GenerateOrder() const;

    std::vector<TBenchmark> CreateBenchmarks() const;

    std::vector<ui64> ConvertToThroughput(const std::vector<ui64>& latencies, ui32 test) const;

private:
    TPattern Pattern;
    std::vector<TFactorLevels> FactorLevels;
    TWarmupParams Warmup;
    TEnvironmentParams Environment;
    ui64 TestDuration;
    ui32 BatchSize;
    ui32 Replays;
    std::vector<std::string> VaryingFactors;
    mutable std::vector<TAPIFactory<TPosixAPI>> APIFactories;
};


#endif
