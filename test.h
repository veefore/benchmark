/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __TEST__H__
#define __TEST__H__


#include "benchmark.h"


class TFixedLatencyAPI : public IAPI {
public:
    virtual std::pair<ssize_t, ui64> pread(int fd, void* buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwrite(int fd, const void *buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

private:
    ui64 Latency = 15_us;
};


class TSwitchingLatencyAPI : public IAPI {
public:
    virtual std::pair<ssize_t, ui64> pread(int fd, void* buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwrite(int fd, const void *buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

private:
    std::vector<ui64> LatencyPool = {20_us, 30_us, 40_us};
    ui64 Index = 0;
};


class TRandomLatencyAPI : public IAPI {
public:
    virtual std::pair<ssize_t, ui64> pread(int fd, void* buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwrite(int fd, const void *buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

private:
    std::vector<ui64> LatencyPool = {10_us, 20_us, 60_us};
};


ui32 CompareResult(const std::vector<ui64>& result, ui64 refMean, ui64 refStd);

ui64 Factorial(ui64 n);

ui64 RandomLatencyStd(const std::vector<ui64>& latencies, ui32 batchSize);

void RunTests();


#endif
