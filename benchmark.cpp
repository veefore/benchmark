/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __BENCHMARK__CPP__
#define __BENCHMARK__CPP__


#include "benchmark.h"

#include <memory> // unique_ptr
#include <chrono> // std::chrono::*
#include <stdexcept> // runtime_error
#include <cmath> // sqrtl()
#include <sys/uio.h> // struct iovec #include <unistd.h> // unlink()
#include <sys/stat.h> // open(), open flags
#include <fcntl.h> // open(), open flags
#include <unistd.h> // close()
#include <tuple> // std::tie()
#include <array> // std::array
#include <cstdio> // popen()

#include <iostream>
using namespace std;


std::pair<ui64, ui64> Statistics(const std::vector<ui64>& sample) {
    if (sample.empty())
        throw std::runtime_error("Trying to get statistics from an empty sample");
    ui64 sum = 0;
    for (ui64 value : sample)
        sum += value;
    ui64 size = sample.size();
    ui64 mean = sum / size;

    sum = 0;
    for (ui64 value : sample)
        sum += (value - mean) * (value - mean);
    ui64 std = (size > 1 ? sqrtl((ld)sum / (size - 1)) : 0);

    return {mean, std};
}


std::string ExecuteCommand(const char* command, ui32& exitStatus) {
    auto pipePtr = popen(command, "r");
    if (!pipePtr)
        throw std::runtime_error("Cannot open pipe.");
    std::array<char, 128> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipePtr) != nullptr)
        result += buffer.data();
    exitStatus = pclose(pipePtr);
    return result;
}


void TFactorLevels::SetLevel(const std::string& factor, ui64 level) {
    if (factor == "RS")
        RequestSize = level;
    else if (factor == "QD")
        QueueDepth = level;
    else if (factor == "DIO")
        DirectIO = level;
    else
        throw runtime_error("TFactorsLevels::SetLevel() error: "
                            "factor " + factor + " not supported");
}


ui64 TFactorLevels::GetLevel(const std::string& factor) const {
    if (factor == "RS")
        return RequestSize;
    else if (factor == "QD")
        return QueueDepth;
    else if (factor == "DIO")
        return DirectIO;
    else
        throw runtime_error("TFactorsLevels::GetLevel() error: "
                            "Factor " + factor + " not supported");
}


TBenchmark::TBenchmark(TPattern pattern,
                       const TFactorLevels& factorLevels,
                       const TWarmupParams& warmup,
                       const TEnvironmentParams& environment,
                       ui64 testDuration,
                       ui32 batchSize,
                       IAPIFactory* factory)
                       : Pattern(pattern)
                       , FactorLevels(factorLevels)
                       , Warmup(warmup)
                       , Environment(environment)
                       , TestDuration(testDuration)
                       , BatchSize(batchSize)
                       , Factory(factory) {}


std::vector<ui64> TBenchmark::Benchmark() {
    // ~ Parameter aliases
    ui64 rs = FactorLevels.RequestSize;
    ui64 qd = FactorLevels.QueueDepth;
    ui32 fd = PrepareEnvironment();
    ui64 filesize = Environment.Filesize;
    IAPI* api = Factory->Construct();

    const ui64 bufSize = ceil((long double)rs / sizeof(ui32));
    // ~ Buffers for storing data used in operations
    std::vector<std::unique_ptr<ui32[]>> bufferPtrs(BatchSize); // Case of qd == 1
    std::vector<std::unique_ptr<struct iovec[]>> iovPtrs(BatchSize); // Case of qd > 1
    // ~ Unique ptrs storing the actual data
    std::vector<std::unique_ptr<ui32[]>> iovData(BatchSize * qd); 

    // Allocate data for buffers.
    for (ui32 i = 0; i < BatchSize; i++) {
        if (qd == 1) {
            bufferPtrs[i].reset(new ui32[bufSize]);
        } else if (qd > 1) {
            iovPtrs[i].reset(new struct iovec[qd]);
            for (ui32 j = 0; j < qd; j++) {
                iovData[i * qd + j] = std::unique_ptr<ui32[]>(new ui32[bufSize]);
                iovPtrs[i][j].iov_base = iovData[i * qd + j].get();
                iovPtrs[i][j].iov_len = rs;
            }
        }
    }

    // ~ Vectors of buffers and iovs used as arguments in read and write operation calls
    std::vector<void *> bufs(BatchSize);
    std::vector<const struct iovec*> iovs(BatchSize);
    for (ui32 i = 0; i < BatchSize; i++) {
        bufs[i] = bufferPtrs[i].get();
        iovs[i] = iovPtrs[i].get();
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
    ui64 measurements = 0;
    while (!warmupDone || Duration(testStart, Nhrc::now()) < TestDuration
            || latencies.size() < MinIterations) {

        ssize_t bytesProcessed;
        ui64 latency;
        if (qd == 1) {
            if (Pattern.IsRead)
                std::tie(bytesProcessed, latency) = api->Read(fd, bufs, rs, offsets);
            else
                std::tie(bytesProcessed, latency) = api->Write(fd, bufs, rs, offsets);
        } else if (qd > 1) {
            if (Pattern.IsRead)
                std::tie(bytesProcessed, latency) = api->Read(fd, iovs, qd, offsets);
            else
                std::tie(bytesProcessed, latency) = api->Write(fd, iovs, qd, offsets);
        }
        latencies.push_back(latency);

        // Make the data different to avoid system optimizations.
        for (ui32 i = 0; i < BatchSize; i++) {
            if (qd == 1) {
                bufferPtrs[i][0]++;
            } else if (qd > 1) {
                for (ui32 j = 0; j < qd; j++)
                    iovData[i * qd + j][0]++;
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
            for (ui32 i = 0; i < Warmup.SampleSize; i++)
                warmupLatencies[i] = latencies[latencies.size() - 1 - i];
            auto [mean, std] = Statistics(warmupLatencies);
            if (Duration(testStart, Nhrc::now()) >= Warmup.MaxDuration
                || std <= Warmup.ThresholdCoef * mean) {
                // cout << "Warmup criterion: " << (std <= Warmup.ThresholdCoef * mean) << endl;
                warmupDone = true;
                latencies.clear();
                testStart = Nhrc::now();
            }
        }
    }
    if (MinIterations == 0)
        MinIterations = latencies.size();

    close(fd);
    unlink(Environment.Filepath.c_str());
    return latencies;
}


ui32 TBenchmark::PrepareEnvironment() const {
    const char* filepath = Environment.Filepath.c_str();
    // Removes the file if it exists and the Unlink flag is set
    if (Environment.Unlink)
        unlink(filepath);

    if (Environment.PreparationScript != "") {
        ui32 exitStatus;
        ExecuteCommand(Environment.PreparationScript.c_str(), exitStatus);
    }

    i32 fd;
    auto flags = O_RDWR | O_CREAT;

    bool dio = FactorLevels.DirectIO;
    #if defined (__linux__)
    if (dio)
        flags |= O_DIRECT;
    #endif

    #if defined (__APPLE__)
    if (dio)
        flags |= F_NOCACHE;
    #endif

    // S_IRWXU = write permission for the file owner
    if ((fd = open(filepath, flags, S_IRWXU)) == -1)
        throw std::runtime_error("Couldn't not open file \"" + Environment.Filepath + "\"");

    ui64 rs = 64_KB;
    ui64 qd = 8;

    srand(time(nullptr));

    // Fill file with random data
    if (Environment.Unlink) {
        ui64 iterations = std::ceil((ld)Environment.Filesize / (rs * qd));
        ui64 bufSize = std::ceil((ld)rs / sizeof(ui32));

        std::unique_ptr<struct iovec[]> iovPtr(new struct iovec[qd]);
        std::vector<std::unique_ptr<ui32[]>> iovData(qd);

        for (ui32 i = 0; i < qd; i++) {
            iovData[i] = std::unique_ptr<ui32[]>(new ui32[bufSize]);
            iovPtr[i].iov_base = iovData[i].get();
            iovPtr[i].iov_len = rs;
        }

        off_t offset = 0;

        IAPI* api = Factory->Construct();
        for (ui64 i = 0; i < iterations; i++) {
            api->Write(fd, {iovPtr.get()}, qd, {offset});
            for (ui32 j = 0; j < qd; j++)
                iovData[j][0]++;
            offset += rs * qd;
        }
    }

    return fd;
}


#endif
