/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __TEST__CPP__
#define __TEST__CPP__


#include "test.h"

#include <cstdlib> // rand()
#include <iostream>
#include <cmath> // sqrtl()


using namespace std;


// ~ Fixed latency API
std::pair<ssize_t, ui64> TFixedLatencyAPI::Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    return {0, Latency * offsets.size()};
}

std::pair<ssize_t, ui64> TFixedLatencyAPI::Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    return {0, Latency * offsets.size()};
}

std::pair<ssize_t, ui64> TFixedLatencyAPI::Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    return {0, Latency * offsets.size()};
}

std::pair<ssize_t, ui64> TFixedLatencyAPI::Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    return {0, Latency * offsets.size()};
}


// ~ Switching latency API
std::pair<ssize_t, ui64> TSwitchingLatencyAPI::Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        Index = (Index + 1) % LatencyPool.size();
        latency += LatencyPool[Index];
    }
    return {0, latency};
}

std::pair<ssize_t, ui64> TSwitchingLatencyAPI::Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        Index = (Index + 1) % LatencyPool.size();
        latency += LatencyPool[Index];
    }
    return {0, latency};
}

std::pair<ssize_t, ui64> TSwitchingLatencyAPI::Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        Index = (Index + 1) % LatencyPool.size();
        latency += LatencyPool[Index];
    }
    return {0, latency};
}

std::pair<ssize_t, ui64> TSwitchingLatencyAPI::Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        Index = (Index + 1) % LatencyPool.size();
        latency += LatencyPool[Index];
    }
    return {0, latency};
}


// ~ Random latency API
std::pair<ssize_t, ui64> TRandomLatencyAPI::Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        ui32 index = rand() % LatencyPool.size();
        latency += LatencyPool[index];
    }
    return {0, latency};
}

std::pair<ssize_t, ui64> TRandomLatencyAPI::Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        ui32 index = rand() % LatencyPool.size();
        latency += LatencyPool[index];
    }
    return {0, latency};
}

std::pair<ssize_t, ui64> TRandomLatencyAPI::Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        ui32 index = rand() % LatencyPool.size();
        latency += LatencyPool[index];
    }
    return {0, latency};
}

std::pair<ssize_t, ui64> TRandomLatencyAPI::Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    ui64 latency = 0;
    for (ui32 i = 0; i < offsets.size(); i++) {
        ui32 index = rand() % LatencyPool.size();
        latency += LatencyPool[index];
    }
    return {0, latency};
}


ui32 CompareResult(const std::vector<ui64>& result, ui64 refMean, ui64 refStd) {
    auto [mean, std] = Statistics(result);
    cout << "Reference mean: " << refMean << endl;
    cout << "Result mean: " << mean << endl;
    cout << "Reference std: " << refStd << endl;
    cout << "Result std: " << std << endl;

    ui64 meanDif = max(refMean, mean) - min(refMean, mean);
    ui64 stdDif = max(refStd, std) - min(refStd, std);

    if (meanDif <= max(1., 0.01 * refMean) && stdDif <= max(1., 0.01 * refStd)) {
        cout << "[✓] Test passed." << endl;
        return 0;
    }
    else {
        cout << "[X] Test failed." << endl;
        return 1;
    }
}

ui64 RandomLatencyStd(const std::vector<ui64>& latencies, ui32 batchSize) {
    // Generate all possible sums of latencies.
    ui64 size = latencies.size();
    vector<ui64> combinationSums;
    vector<ui32> indices(batchSize, 0);
    while (true) {
        combinationSums.push_back(0);
        for (ui32 j = 0; j < batchSize; j++)
            combinationSums.back() += latencies[indices[j]];
        for (ui32 j = batchSize - 1; j < batchSize && j >= 0; j--) {
            indices[j]++;
            if (indices[j] == size)
                indices[j] = 0;
            else
                break;
        }
        bool flag = true;
        for (ui32 j = 0; j < batchSize; j++)
            if (indices[j] != 0)
                flag = false;
        if (flag)
            break;
    }

    ui64 sum = 0;
    for (ui64 val : combinationSums)
        sum += val;
    ui64 mean = sum / combinationSums.size();

    sum = 0;
    for (ui64 val : combinationSums)
        sum += (val - mean) * (val - mean);
    ui64 std = sqrtl((sum * 1000000) / (combinationSums.size() * 999999));

    return std;
}

void RunTests() {
    // ~ Test params
    TPattern pattern;
    pattern.IsConsecutive = true;
    pattern.IsRead = true;

    TFactors factors;
    factors.RequestSize = 1_KB;
    factors.QueueDepth = 1;

    TWarmupParams warmup;
    warmup.ThresholdCoef = 0.15;
    warmup.MaxDuration = 1_s;
    warmup.SampleSize = 100;

    TEnvironmentParams environment;
    environment.Filepath = "testfile";
    environment.Filesize = 512_MB;

    ui64 testDuration = 10_s;
    vector<ui32> batchSizes = {1, 5, 10};
    ui32 sizes = batchSizes.size();


    // ~ Reference test results
    vector<ui64> fixedLatencyMean(sizes, 15); // mean = 15 * batchSize
    vector<ui64> fixedLatencyStd(sizes, 0); // 0

    vector<ui64> switchingLatencyMean(sizes, 30); // 30 * batchSize
    vector<ui64> switchingLatencyStd(sizes, 8); // 8 | Static because there is no randomness.

    vector<ui64> randomLatencyMean(sizes, 30); // 30 * batchSize
    vector<ui64> latencies = {10_us, 20_us, 60_us};
    vector<ui64> randomLatencyStd(sizes);

    for (ui32 i = 0; i < sizes; i++) {
        fixedLatencyMean[i] *= batchSizes[i];
        switchingLatencyMean[i] *= batchSizes[i];
        randomLatencyMean[i] *= batchSizes[i];
        
        randomLatencyStd[i] = RandomLatencyStd(latencies, batchSizes[i]);
    }
    

    ui32 failed = 0;
    for (ui32 i = 0; i < sizes; i++) {
        cout << "-----------------------------------" << endl;
        cout << "Testing with batch size: " << batchSizes[i] << endl;
        cout << "-----------------------------------" << endl;

        cout << "Fixed latency test. Latency: 15 us." << endl;
        TAPIFactory<TFixedLatencyAPI> fixedFactory;
        TBenchmark fixedBenchmark(pattern, factors, warmup, environment, testDuration, batchSizes[i], &fixedFactory);
        failed += CompareResult(fixedBenchmark.Benchmark(), fixedLatencyMean[i], fixedLatencyStd[i]);
        cout << endl;

        cout << "Switching latency test. Latencies: {20 us, 30 us, 40 us}." << endl;
        TAPIFactory<TSwitchingLatencyAPI> switchingFactory;
        TBenchmark switchingBenchmark(pattern, factors, warmup, environment, testDuration, batchSizes[i], &switchingFactory);    
        failed += CompareResult(switchingBenchmark.Benchmark(), switchingLatencyMean[i], switchingLatencyStd[i]);
        cout << endl;

        cout << "Random latency test. Latencies: {10 us, 20 us, 60 us}." << endl;
        TAPIFactory<TRandomLatencyAPI> randomFactory;
        TBenchmark randomBenchmark(pattern, factors, warmup, environment, testDuration, batchSizes[i], &randomFactory);
        failed += CompareResult(randomBenchmark.Benchmark(), randomLatencyMean[i], randomLatencyStd[i]);
        cout << endl;

        cout << endl;
    }
    cout << "Test passed: " << (sizes * 3 - failed) << "/" << (sizes * 3) << endl;
    if (failed == 0)
        cout << "Success." << endl;
    else
        cout << "Fail." << endl;
}


#endif
