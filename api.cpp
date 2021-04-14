/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __API__CPP__
#define __API__CPP__


#include "api.h"

#include <sys/types.h> // lseek()
#include <sys/uio.h> // readv(), writev()
#include <unistd.h> // pread(), pwrite()

using Nhrc = std::chrono::high_resolution_clock;

std::pair<ssize_t, ui64> TPosixAPI::pread(int fd, void* buf, size_t count, off_t offset) {
    auto start = Nhrc::now();
    ssize_t bytesProcessed = ::pread(fd, buf, count, offset);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}

std::pair<ssize_t, ui64> TPosixAPI::pwrite(int fd, const void *buf, size_t count, off_t offset) {
    auto start = Nhrc::now();
    ssize_t bytesProcessed = ::pwrite(fd, buf, count, offset);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}

std::pair<ssize_t, ui64> TPosixAPI::preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) {
    auto start = Nhrc::now();
    lseek(fd, offset, SEEK_SET);
    ssize_t bytesProcessed = ::readv(fd, iov, iovcnt);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}

std::pair<ssize_t, ui64> TPosixAPI::pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) {
    auto start = Nhrc::now();
    lseek(fd, offset, SEEK_SET);
    ssize_t bytesProcessed = ::writev(fd, iov, iovcnt);
    auto end = Nhrc::now();
    return {bytesProcessed, Duration(start, end)};
}



#endif
