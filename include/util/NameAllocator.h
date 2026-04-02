#ifndef _GENERIC_UTIL_NAME_ALLOCATOR_H
#define _GENERIC_UTIL_NAME_ALLOCATOR_H

#include <map>

#include <cstdint>

namespace Generic::Util
{
using Name   = std::uint64_t;
using Unused = int;

class NameAllocator final
{
public:
    NameAllocator() : maxCount(1000)
    {
    }

    NameAllocator(int _maxCount) : maxCount(_maxCount)
    {
    }

    [[nodiscard]] Name                    getName();
    void                                  returnName(Name name);
    [[nodiscard]] bool                    isReserved(Name name) noexcept;
    [[nodiscard]] bool                    noNameReserved() noexcept;
    [[nodiscard]] Name                    lastNameReserved() noexcept;
    [[nodiscard]] std::map<Name, Unused>& data() noexcept;

private:
    unsigned int           maxCount;
    Name                   firstUnusedName = 0;
    std::map<Name, Unused> restoredNames;
    std::map<Name, Unused> allocatedNames;
};
} // namespace Generic::Util

#endif /* GENERIC_UTIL_NAME_ALLOCATOR_HPP */
