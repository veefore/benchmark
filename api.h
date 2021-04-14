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

    virtual std::pair<ssize_t, ui64> pread(int fd, void* buf, size_t count, off_t offset) = 0;

    virtual std::pair<ssize_t, ui64> pwrite(int fd, const void *buf, size_t count, off_t offset) = 0;

    virtual std::pair<ssize_t, ui64> preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) = 0;

    virtual std::pair<ssize_t, ui64> pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) = 0;
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
        APIs.emplace_back();
        return &APIs.back();
    }

    std::vector<IAPI*> Construct(ui32 amount) override {
        APIs.resize(APIs.size() + amount);
        std::vector<IAPI*> result(amount);
        for (ui32 i = 0; i < amount; i++) {
            result[amount - 1 - i] = &APIs[APIs.size() - 1 - i];
        }
        return result;
    }

private:
    std::vector<TAPI> APIs;
};


// ~ API interface POSIX implementation
class TPosixAPI : public IAPI {
public:
    virtual std::pair<ssize_t, ui64> pread(int fd, void* buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwrite(int fd, const void *buf, size_t count, off_t offset) override;

    virtual std::pair<ssize_t, ui64> preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

    virtual std::pair<ssize_t, ui64> pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) override;

};


#endif
