#ifndef _TRIMEGO_MATH_H
#define _TRIMEGO_MATH_H

namespace Trimego::Util
{
    #if defined(__x86_64__) || defined(_M_AMD64)
#include <intrin.h>

#define base2log(x)                                                            \
    (_BitScanReverse64(reinterpret_cast<unsigned long*>(&x), x), (int)x)

#elif defined(__aarch64__) || defined(_M_ARM64)

#define base2log(x) 63 - __builtin_clzll(x)

#endif
}

#endif