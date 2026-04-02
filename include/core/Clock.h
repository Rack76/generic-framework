#ifndef _TRIMEGO_CLOCK_H
#define _TRIMEGO_CLOCK_H

#include "util/Singleton.h"

namespace Generic::Util
{
class Clock final : public Singleton<Clock>
{
public:
    unsigned long count = 0;
};
} // namespace Generic::Util

#endif