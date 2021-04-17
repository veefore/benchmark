/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __API__CPP__
#define __API__CPP__


#include "api.h"

#include <sys/types.h> // lseek()
#include <sys/uio.h> // readv(), writev()
#include <unistd.h> // pread(), pwrite()

using Nhrc = std::chrono::high_resolution_clock;


// ~ IAPI batch operations with time measurement
std::pair<ssize_t, ui64> IAPI::Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    auto start = Nhrc::now();
    ssize_t bytesProcessed = 0;
    for (ui32 i = 0; i < bufs.size(); i++)
        bytesProcessed += pread(fd, bufs[i], count, offsets[i]);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}

std::pair<ssize_t, ui64> IAPI::Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets) {
    auto start = Nhrc::now();
    ssize_t bytesProcessed = 0;
    for (ui32 i = 0; i < bufs.size(); i++)
        bytesProcessed += pwrite(fd, bufs[i], count, offsets[i]);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}

std::pair<ssize_t, ui64> IAPI::Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    auto start = Nhrc::now();
    ssize_t bytesProcessed = 0;
    for (ui32 i = 0; i < iovs.size(); i++)
        bytesProcessed += preadv(fd, iovs[i], iovcnt, offsets[i]);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}

std::pair<ssize_t, ui64> IAPI::Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets) {
    auto start = Nhrc::now();
    ssize_t bytesProcessed = 0;
    for (ui32 i = 0; i < iovs.size(); i++)
        bytesProcessed += pwritev(fd, iovs[i], iovcnt, offsets[i]);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}


// ~ TPosixAPI base operations
ssize_t TPosixAPI::pread(int fd, void* buf, size_t count, off_t offset) {
    return ::pread(fd, buf, count, offset);
}

ssize_t TPosixAPI::pwrite(int fd, const void *buf, size_t count, off_t offset) {
    return ::pwrite(fd, buf, count, offset);
}

ssize_t TPosixAPI::preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) {
    lseek(fd, offset, SEEK_SET);
    return ::readv(fd, iov, iovcnt);
}

ssize_t TPosixAPI::pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) {
    lseek(fd, offset, SEEK_SET);
    return ::writev(fd, iov, iovcnt);
}

#endif
