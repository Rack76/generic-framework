#include "util/NameAllocator.h"
#include <iostream>
#include <limits>

namespace Trimego::Util
{
Name
NameAllocator::getName()
{
    if (restoredNames.size() == 0)
    {
        if (firstUnusedName != maxCount)
        {
            allocatedNames.insert({firstUnusedName, firstUnusedName});
            return firstUnusedName++;
        }
        else
        {
            std::cerr << "name not allocated : max count exceeded";
            return -1;
        }
    }
    else
    {
        auto it          = restoredNames.begin();
        int  returnValue = it->first;
        restoredNames.erase(returnValue);
        allocatedNames.insert({returnValue, returnValue});
        return returnValue;
    }
}

void
NameAllocator::returnName(Name name)
{
    if (allocatedNames.find(name) != allocatedNames.end())
    {
        allocatedNames.erase(name);
        restoredNames.insert({name, name});
    }
    else
        std::cerr << "name not returned : name not allocated";
}

bool
NameAllocator::isReserved(Name name) noexcept
{
    if (allocatedNames.find(name) != allocatedNames.end())
        return true;
    return false;
}

inline bool
NameAllocator::noNameReserved() noexcept
{
    return allocatedNames.size() == 0;
}

Name
NameAllocator::lastNameReserved() noexcept
{
    if (allocatedNames.size() == 0)
        return -1;
    auto it = allocatedNames.end();
    return std::prev(it)->first;
}

inline std::map<Name, Unused>&
NameAllocator::data() noexcept
{
    return allocatedNames;
}

} // namespace Trimego::Util