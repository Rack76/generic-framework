#ifndef _TRIMEGO_UTIL_OBJECT_POOL_H
#define _TRIMEGO_UTIL_OBJECT_POOL_H

#include <cstring>
#include <iostream>
#include <new>
#include <stdint.h>
#include <unordered_map>
#include "util/NameAllocator.h"

using BlockHandle = std::uint64_t;
using ObjectId    = std::uint64_t;

namespace Generic::Core
{
class ObjectPool
{
public:
    /**
     * @brief reserves an object, initializes its value.
     * @param blockHandle a handle to a contiguous section of memory containing objects.
     * @param value a pointer to the value to initialize the object with.
     * @return a pointer to the reserved object.
     * @note block handles are used to maintain contiguity of the components when processing
     * archetypes.
     */
    [[nodiscard]] virtual void* getObject(BlockHandle blockHandle, void* value) = 0;

    /**
     * @brief reserves an object.
     * @param blockHandle a handle to a contiguous section of memory containing objects.
     * @return a pointer to the reserved object.
     * @note block handles are used to maintain contiguity of the components when processing
     * archetypes.
     */
    [[nodiscard]] virtual void* getObject(BlockHandle blockHandle) = 0;

    /**
     * @brief copy an object to its destination in memory.
     * @param blockHandle a handle to a contiguous section of memory containing objects.
     * @param dest a pointer to the destination to copy the object to.
     * @param source a pointer to the source object.
     * @note block handles are used to maintain contiguity of the components when processing
     * archetypes.
     */
    virtual void copy(BlockHandle blockHandle, void* dest, void* source) = 0;

    /**
     * @brief frees an reserved object.
     * @param blockHandle a handle to a contiguous section of memory containing objects.
     * @param object a pointer to the object to free.
     * @note block handles are used to maintain contiguity of the components when processing
     * archetypes.
     */
    inline virtual void returnObject(BlockHandle blockHandle, void* object) = 0;

    virtual ~ObjectPool()
    {
    }
};

template <typename T>
class ObjectPoolTemplate final : public ObjectPool
{
public:
    ObjectPoolTemplate()
    {
        size_t size = sizeof(T);
        data        = new T*[blockCount];
        for (int i = 0; i < blockCount; i++)
        {
            data[i] = new T[blockSize]();
        }
    }

    virtual void* getObject(BlockHandle blockHandle, void* value) noexcept
    {
        int objectId = nameAllocators[blockHandle].getName();
        T&  object   = data[blockHandle][objectId];
        object       = *reinterpret_cast<T*>(value);
        objectIds[reinterpret_cast<std::uintptr_t>(&object)] = objectId;
        return reinterpret_cast<void*>(&object);
    }

    virtual void* getObject(BlockHandle blockHandle) noexcept
    {
        int objectId = nameAllocators[blockHandle].getName();
        T&  object = data[blockHandle][objectId]             = T();
        objectIds[reinterpret_cast<std::uintptr_t>(&object)] = objectId;
        return reinterpret_cast<void*>(&object);
    }

    virtual void copy(BlockHandle blockHandle, void* destPtr, void* sourcePtr) noexcept
    {
        ObjectId sourceId         = objectIds[reinterpret_cast<std::uintptr_t>(sourcePtr)];
        T&       source           = data[blockHandle][sourceId];
        ObjectId destId           = objectIds[reinterpret_cast<std::uintptr_t>(destPtr)];
        data[blockHandle][destId] = source;
    }

    inline virtual void returnObject(BlockHandle blockHandle, void* object) noexcept
    {
        nameAllocators[blockHandle].returnName(objectIds[reinterpret_cast<std::uintptr_t>(object)]);
    }

    ~ObjectPoolTemplate() noexcept
    {
        for (int i = 0; i < blockCount; i++)
        {
            delete[] data[i];
        }
        delete[] data;
    }

private:
    int getObjectOffset(ObjectId objectId, BlockHandle blockHandle) noexcept
    {
        return blockHandle * blockSize + objectId;
    }

    static constexpr size_t blockCount = 5; // set the first two variables as you like
    static constexpr size_t blockSize  = 64;
    static constexpr size_t poolSize   = blockCount * blockSize;
    void*                   bytes;
    T**                     data;
    std::unordered_map<BlockHandle, Util::NameAllocator> nameAllocators;
    std::unordered_map<std::uintptr_t, ObjectId>         objectIds;
};
} // namespace Generic::Core

#endif /* TRIMEGO_UTIL_OBJECT_POOL_HPP */
