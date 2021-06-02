/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __API__H__
#define __API__H__


#include "globals.h"

#include <sys/types.h>
#include <vector>
#include <chrono>
#include <utility> // std::pair


// ~ API interface
class IAPI {
public:
    virtual ~IAPI() = default;

    // ~ Batch operations with time measurement
    // ~ Positional
    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets);

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<void*>& bufs, size_t count, const std::vector<off_t>& offsets);

    // ~ Positional and vectored
    virtual std::pair<ssize_t, ui64> Read(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets);

    virtual std::pair<ssize_t, ui64> Write(int fd, const std::vector<const struct iovec*>& iovs, int iovcnt, const std::vector<off_t>& offsets);

protected:
    // ~ Base operations
    virtual ssize_t pread(int fd, void* buf, size_t count, off_t offset) = 0;

    virtual ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) = 0;

    virtual ssize_t preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) = 0;

    virtual ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) = 0;

};


// ~ API factory interface
// Constructs and stores IAPI* objects
class IAPIFactory {
public:
    virtual ~IAPIFactory() = default;

    virtual IAPI* Construct() = 0;

    virtual std::vector<IAPI*> Construct(ui32 amount) = 0;
};


// ~ API factory interface implementation
template <typename TAPI>
class TAPIFactory : public IAPIFactory {
public:
    ~TAPIFactory() { APIs.clear(); }

    IAPI* Construct() override {
        APIs.clear();
        APIs.emplace_back();
        return &APIs.back();
    }

    std::vector<IAPI*> Construct(ui32 amount) override {
        APIs.clear();
        APIs.resize(amount);
        std::vector<IAPI*> result(amount);
        for (ui32 i = 0; i < amount; i++) {
            result[i] = &APIs[i];
        }
        return result;
    }

private:
    std::vector<TAPI> APIs;
};


// ~ API interface POSIX implementation
class TPosixAPI : public IAPI {
public:
    virtual ssize_t pread(int fd, void* buf, size_t count, off_t offset) override;

    virtual ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) override;

    virtual ssize_t preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

    virtual ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

};


#endif
