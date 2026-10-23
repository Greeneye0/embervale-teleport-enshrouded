#pragma once
#include "countdown.h"
namespace teleport {
inline bool ArrivedAt(Destination actual,Destination expected) {
    constexpr long double tolerance=4294967296.0L/100; // one centimetre
    return std::fabs(static_cast<long double>(actual.x)-expected.x)<=tolerance&&
           std::fabs(static_cast<long double>(actual.y)-expected.y)<=tolerance&&
           std::fabs(static_cast<long double>(actual.z)-expected.z)<=tolerance;
}
}
