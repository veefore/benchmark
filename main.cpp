/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __MAIN__CPP__
#define __MAIN__CPP__


#include "benchmark.h"

#include <iostream>


using namespace std;


int main() {
    TPattern pattern;
    pattern.IsConsecutive = false;
    pattern.IsRead = false;

    TFactors factors;
    factors.RequestSize = 8_KB;
    factors.QueueDepth = 1;

    TWarmupParams warmup;
    warmup.ThresholdCoef = 0.15;
    warmup.MaxDuration = 5_s;
    warmup.SampleSize = 100;

    TEnvironmentParams environment;
    environment.Filepath = "testfile";
    environment.Filesize = 512_MB;

    ui64 testDuration = 10_s;
    ui32 batchSize = 5;

    TAPIFactory<TPosixAPI> factory;

    TBenchmark benchmark(pattern, factors, warmup, environment, testDuration, batchSize, &factory);
    
    auto result = benchmark.Benchmark();
    auto [mean, std] = Statistics(result);
    cout << "mean " << mean << endl;
    cout << "std " << std << endl;
    return 0;
}


#endif
