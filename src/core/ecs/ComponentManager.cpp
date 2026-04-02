#include "core/ecs/ComponentManager.h"
#include <iostream>

namespace Trimego::Core::ECS
{
BlockHandle
ComponentManager::getBlockHandle(ComponentTypeId componentTypeId)
{
    BlockHandle blockHandle = blockHandleCounters[componentTypeId]++;
    return blockHandle;
}

void*
ComponentManager::getComponent(BlockHandle block, ComponentTypeId componentTypeId, void* value)
{
#ifndef NDEBUG
    if (pools.find(componentTypeId) != pools.end())
        std::cout << "In ComponentManager::getPoolHandle:: component type not registered"
                  << std::endl;
#endif

    void* c = pools[componentTypeId].get()->getObject(block, value);
    return c;
}

void*
ComponentManager::getComponent(BlockHandle block, ComponentTypeId componentTypeId)
{
#ifndef NDEBUG
    if (pools.find(componentTypeId) != pools.end())
        std::cout << "In ComponentManager::getPoolHandle:: component type not registered"
                  << std::endl;
#endif

    void* c = pools[componentTypeId].get()->getObject(block);
    return c;
}

void
ComponentManager::copyComponent(BlockHandle     block,
                                ComponentTypeId componentTypeId,
                                void*           dest,
                                void*           source) noexcept
{
#ifndef NDEBUG
    if (pools.find(componentTypeId) != pools.end())
        std::cout << "In ComponentManager::getPoolHandle:: component type not registered"
                  << std::endl;
#endif

    pools[componentTypeId].get()->copy(block, dest, source);
}

void
ComponentManager::returnComponent(ComponentTypeId componentTypeId,
                                  BlockHandle     blockHandle,
                                  void*           component) noexcept
{
#ifndef NDEBUG
    if (pools.find(componentTypeId) != pools.end())
        std::cout << "In ComponentManager::getPoolHandle:: component type not registered"
                  << std::endl;
#endif

    pools[componentTypeId].get()->returnObject(blockHandle, component);
}

std::unordered_map<std::uint64_t, std::shared_ptr<ObjectPool>> ComponentManager::pools;
std::unordered_map<ComponentTypeId, BlockHandle> ComponentManager::blockHandleCounters;
std::unordered_map<ComponentTypeId, std::string> ComponentManager::typeNames;
} // namespace Trimego::Core::ECS