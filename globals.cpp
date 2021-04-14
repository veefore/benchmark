/* Copyright © 2021 Vladimir Erofeev. All rights reserved. */

#ifndef __GLOBALS__CPP__
#define __GLOBALS__CPP__

#include "globals.h"

#include <cstdlib> // rand()


ui32 RandomUI32() {
    return (rand() % 32768) * (rand() % 32768);
}

ui64 Duration(const TTimePoint& lhs, const TTimePoint& rhs) {
    return std::chrono::duration_cast<std::chrono::microseconds>(rhs - lhs).count();
}


#endif
