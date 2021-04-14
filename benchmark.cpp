/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __BENCHMARK__CPP__
#define __BENCHMARK__CPP__


#include "benchmark.h"

#include <memory> // unique_ptr
#include <chrono> // std::chrono::*
#include <numeric> // std::accumulate()
#include <stdexcept> // runtime_error
#include <cmath> // sqrtl()
#include <sys/uio.h> // struct iovec
#include <unistd.h> // unlink()
#include <sys/stat.h> // open(), open flags
#include <fcntl.h> // open(), open flags
#include <tuple> // std::tie()

#include <iostream>
using namespace std;


TBenchmark::TBenchmark(const TPattern& pattern, const TFactors& factors,
                       const TWarmupParams& warmup,
                       const TEnvironmentParams& environment, ui64 testDuration,
                       ui32 batchSize, IAPIFactory* factory)
                       : Pattern(pattern)
                       , Factors(factors)
                       , Warmup(warmup)
                       , Environment(environment)
                       , TestDuration(testDuration)
                       , BatchSize(batchSize)
                       , Factory(factory) {}


std::vector<ui64> TBenchmark::Benchmark() {
    // ~ Parameter aliases
    ui64 rs = Factors.RequestSize;
    ui64 qd = Factors.QueueDepth;
    ui32 fd = PrepareEnvironment();
    ui64 filesize = Environment.Filesize;
    IAPI* api = Factory->Construct();

    const ui64 bufSize = ceil((long double)rs / sizeof(ui32));
    // ~ Buffers for storing data used in operations
    std::vector<std::unique_ptr<ui32[]>> bufferPtrs(BatchSize); // Case of qd == 1
    std::vector<std::unique_ptr<struct iovec[]>> buffersPtrs(BatchSize); // Case of qd > 1
    // ~ Unique ptrs storing the actual data
    std::vector<std::unique_ptr<ui32[]>> buffersData(BatchSize * qd); 

    // Allocate data for buffers.
    for (ui32 i = 0; i < BatchSize; i++) {
        if (qd == 1) {
            bufferPtrs[i].reset(new ui32[bufSize]);
        } else if (qd > 1) {
            buffersPtrs[i].reset(new struct iovec[qd]);
            for (ui32 j = 0; j < qd; j++) {
                buffersData[i * BatchSize + j] = std::unique_ptr<ui32[]>(new ui32[bufSize]);
                buffersPtrs[i][j].iov_base = buffersData[i * BatchSize + j].get();
                buffersPtrs[i][j].iov_len = rs;
            }
        }
    }

    // ~ Batch latencies
    std::vector<ui64> latencies;

    // ~ File offsets for each operation in batch
    std::vector<off_t> offsets(BatchSize, 0);
    // Set offsets for the first batch.
    if (Pattern.IsConsecutive) {
        for (ui32 i = 1; i < BatchSize; i++)
            offsets[i] = offsets[i - 1] + rs * qd;
    }
    else {
        for (ui32 i = 0; i < BatchSize; i++)
            offsets[i] = RandomUI32() % (filesize - rs * qd);
    }

    auto testStart = Nhrc::now();
    bool warmupDone = false;
    while (!warmupDone || Duration(testStart, Nhrc::now()) < TestDuration) {
        ui64 batchLatency = 0;
        for (ui32 i = 0; i < BatchSize; i++) { 
           ssize_t bytesProcessed;
           ui64 latency;
           if (qd == 1) {
                if (Pattern.IsRead)
                    std::tie(bytesProcessed, latency) = api->pread(fd, bufferPtrs[i].get(), rs, offsets[i]);
                else
                    std::tie(bytesProcessed, latency) = api->pwrite(fd, bufferPtrs[i].get(), rs, offsets[i]);
            } else if (qd > 1) {
                if (Pattern.IsRead)
                    std::tie(bytesProcessed, latency) = api->preadv(fd, buffersPtrs[i].get(), qd, offsets[i]);
                else
                    std::tie(bytesProcessed, latency) = api->pwritev(fd, buffersPtrs[i].get(), qd, offsets[i]);
            }
            batchLatency += latency;
        }
        latencies.push_back(batchLatency);

        // Make the data different to avoid system optimizations.
        for (ui32 i = 0; i < BatchSize; i++) {
            if (qd == 1) {
                bufferPtrs[i][0]++;
            } else if (qd > 1) {
                for (ui32 j = 0; j < qd; j++)
                    buffersData[i * BatchSize + j][0]++;
            }
        }

        // Set offsets for the next batch.
        for (ui32 i = 0; i < BatchSize; i++) {
            if (Pattern.IsConsecutive)
                offsets[i] = (offsets[i] + rs * qd * BatchSize) % filesize;
            else
                offsets[i] = RandomUI32() % (filesize - rs * qd);
        }

        if (!warmupDone) {
            if (latencies.size() < Warmup.SampleSize)
                continue;
            std::vector<ui64> warmupLatencies(Warmup.SampleSize);
            for (ui32 i = 0; i < 10; i++)
                warmupLatencies[i] = latencies[latencies.size() - 1 - i];
            auto [mean, std] = Statistics(warmupLatencies);
            if (Duration(testStart, Nhrc::now()) >= Warmup.MaxDuration
                || std <= Warmup.ThresholdCoef * mean) {
                cout << "Warmup criterion: " << (std <= Warmup.ThresholdCoef * mean) << endl;
                warmupDone = true;
                latencies.clear();
                testStart = Nhrc::now();
            }
        }
    }
    return latencies;
}


ui32 TBenchmark::PrepareEnvironment() {
    const char* filepath = Environment.Filepath.c_str();
    // Removes the file if it exists and the Unlink flag is set
    if (Environment.Unlink)
        unlink(filepath);

    i32 fd;
    auto flags = O_RDWR | O_CREAT;
    // S_IRWXU = write permission for the file owner
    if ((fd = open(filepath, flags, S_IRWXU)) == -1)
        throw std::runtime_error("Couldn't not open file");

    ui64 rs = Factors.RequestSize;
    ui64 qd = Factors.QueueDepth;

    srand(time(nullptr));
    ui64 iterations = std::ceil((ld)Environment.Filesize / (rs * qd));
    ui64 bufSize = std::ceil((ld)rs / sizeof(ui32));

    std::unique_ptr<struct iovec[]> buffersPtr(new struct iovec[qd]);
    std::vector<std::unique_ptr<ui32[]>> buffersData(qd);

    for (ui32 i = 0; i < qd; i++) {
        buffersData[i] = std::unique_ptr<ui32[]>(new ui32[bufSize]);
        buffersPtr[i].iov_base = buffersData[i].get();
        buffersPtr[i].iov_len = rs;
    }

    off_t offset = 0;

    IAPI* api = Factory->Construct();
    for (ui64 i = 0; i < iterations; i++) {
        api->pwritev(fd, buffersPtr.get(), qd, offset);
        for (ui32 j = 0; j < qd; j++)
            buffersData[j][0]++;
        offset += rs * qd;
    }

    return fd;
}


std::pair<ui64, ui64> Statistics(const std::vector<ui64>& sample) {
    if (sample.empty())
        throw std::runtime_error("Trying to get statistics from an empty sample");
    ui64 sum = std::accumulate(sample.begin(), sample.end(), 0);
    ui32 size = sample.size();
    ui64 mean = floor(sum / size);

    sum = 0;
    for (ui64 value : sample)
        sum += (value - mean) * (value - mean);
    ui64 std = (size > 1 ? floor(sqrtl(sum / (size - 1))) : 0);

    return {mean, std};
}


#endif