/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __EXPERIMENTER__CPP__
#define __EXPERIMENTER__CPP__


#include "experimenter.h"

#include <stdexcept> // std::runtime_error()
#include <random> // std::random_device, std::mt19937
#include <algorithm> // std::shuffle()

//!!
#include <iostream>



void AddVectors(std::vector<ui64>& result, const std::vector<ui64>& toAdd) {
    if (result.size() == 0)
        result.resize(toAdd.size(), 0);

    // toAdd vector may be bigger, but cannot be smaller
    if (result.size() > toAdd.size())
        throw std::runtime_error("First vector is bigger than the second one");

    for (ui64 i = 0; i < result.size(); i++)
        result[i] += toAdd[i];
}


TExperimenter::TExperimenter(TPattern pattern,
                             std::vector<TFactorLevels>&& factorLevels,
                             const TWarmupParams& warmup,
                             const TEnvironmentParams& environment,
                             ui64 testDuration,
                             ui32 batchSize,
                             ui32 replays,
                             const std::vector<std::string>& varyingFactors)
                             : Pattern(pattern)
                             , FactorLevels(factorLevels)
                             , Warmup(warmup)
                             , Environment(environment)
                             , TestDuration(testDuration)
                             , BatchSize(batchSize)
                             , Replays(replays)
                             , VaryingFactors(varyingFactors)
                             , APIFactories(factorLevels.size()) {}


std::vector<std::pair<ui64, ui64>> TExperimenter::Experiment() const {
    std::cerr << "\nStarting experiment\n";
    std::vector<std::vector<ui64>> testResults(FactorLevels.size(), std::vector<ui64>());
    std::vector<ui32> order = GenerateOrder();
    std::vector<TBenchmark> benchmarks = CreateBenchmarks();

    for (ui32 i = 0; i < order.size(); i++) {
        auto result = benchmarks[order[i]].Benchmark();
        AddVectors(testResults[order[i]], ConvertToThroughput(result, order[i]));
        std::cerr << "Finished test: " << (i + 1) << "/" << order.size() << "\n";
    }

    for (auto& result : testResults)
        for (auto& value : result)
            value /= Replays;

    std::vector<std::pair<ui64, ui64>> resultStatistics(testResults.size());
    for (ui32 i = 0; i < testResults.size(); i++)
        resultStatistics[i] = Statistics(testResults[i]);

    return resultStatistics;
}


TPattern TExperimenter::GetPattern() const {
    return Pattern;
}


const std::vector<TFactorLevels>& TExperimenter::GetFactorLevels() const {
    return FactorLevels;
}


const std::vector<std::string>& TExperimenter::GetVaryingFactors() const {
    return VaryingFactors;
}


std::vector<ui32> TExperimenter::GenerateOrder() const {
    ui64 tests = FactorLevels.size();
    std::vector<ui32> order(tests * Replays);
    for (ui32 i = 0; i < tests * Replays; i++)
        order[i] = i % tests;
    srand(time(nullptr));
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(order.begin(), order.end(), g);
    return order;
}


std::vector<TBenchmark> TExperimenter::CreateBenchmarks() const {
    std::vector<TBenchmark> benchmarks;
    for (ui32 test = 0; test < FactorLevels.size(); test++) {
        TBenchmark benchmark(Pattern, FactorLevels[test], Warmup, Environment, TestDuration, BatchSize, &APIFactories[test]);
        benchmarks.push_back(benchmark);
    }
    return benchmarks;
}


std::vector<ui64> TExperimenter::ConvertToThroughput(const std::vector<ui64>& latencies, ui32 test) const {
    std::vector<ui64> throughputs(latencies.size());
    ui64 rs = FactorLevels[test].RequestSize;
    ui64 qd = FactorLevels[test].QueueDepth;
    for (ui64 i = 0; i < latencies.size(); i++)
        throughputs[i] = BatchSize * rs * qd * 1_s / latencies[i];
    return throughputs;
}


#endif
