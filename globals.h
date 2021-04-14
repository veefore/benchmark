/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __GLOBALS__H__
#define __GLOBALS__H__


#include <cstdint>
#include <chrono>


using i32 = int32_t;
using ui32 = uint32_t;
using ui64 = uint64_t;
using ull = unsigned long long int;
using ld = long double;

using Nhrc = std::chrono::high_resolution_clock;
using TTimePoint = Nhrc::time_point;


constexpr ui64 operator"" _B(ull value) {
    return value;
}

constexpr ui64 operator"" _KB(ull value) {
    return value * 1000;
}

constexpr ui64 operator"" _MB(ull value) {
    return value * 1000 * 1000;
}

constexpr ui64 operator"" _GB(ull value) {
    return value * 1000 * 1000 * 1000;
}


constexpr ui64 operator"" _us(ull value) {
    return value;
}

constexpr ui64 operator"" _ms(ull value) {
    return value * 1000;
}

constexpr ui64 operator"" _s(ull value) {
    return value * 1000 * 1000;
}

constexpr ui64 operator"" _min(ull value) {
    return value * 1000 * 1000 * 60;
}

 
ui32 RandomUI32();

ui64 Duration(const TTimePoint& lhs, const TTimePoint& rhs);


#endif
