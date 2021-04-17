/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __TEST__H__
#define __TEST__H__


#include "benchmark.h"


class TFixedLatencyAPI : public IAPI {
public:
    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) override;

private:
    virtual ssize_t pread(int fd, void* buf, size_t count, off_t offset) override { return 0; }

    virtual ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) override { return 0; }

    virtual ssize_t preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override { return 0; }

    virtual ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override { return 0; }

    ui64 Latency = 15_us;
};


class TSwitchingLatencyAPI : public IAPI {
public:
    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) override;

private:
    virtual ssize_t pread(int fd, void* buf, size_t count, off_t offset) override { return 0; }

    virtual ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) override { return 0; }

    virtual ssize_t preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override { return 0; }

    virtual ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override { return 0; }

    std::vector<ui64> LatencyPool = {20_us, 30_us, 40_us};
    ui64 Index = 0;
};


class TRandomLatencyAPI : public IAPI {
public:
    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) override;

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) override;

private:
    virtual ssize_t pread(int fd, void* buf, size_t count, off_t offset) override { return 0; }

    virtual ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) override { return 0; }

    virtual ssize_t preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override { return 0; }

    virtual ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override { return 0; }

    std::vector<ui64> LatencyPool = {10_us, 20_us, 60_us};
};


ui32 CompareResult(const std::vector<ui64>& result, ui64 refMean, ui64 refStd);

ui64 RandomLatencyStd(const std::vector<ui64>& latencies, ui32 batchSize);

void RunTests();


#endif
