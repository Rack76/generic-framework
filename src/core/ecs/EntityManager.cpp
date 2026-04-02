#include "core/ecs/EntityManager.h"

namespace Generic::Core::ECS
{
std::vector<EntityId>&
EntityManager::getAddedComponents(ComponentTypeId type)
{
    return componentsAdded[type];
}

std::vector<EntityId>&
EntityManager::getRemovedComponents(ComponentTypeId type)
{
    return componentsRemoved[type];
}

void
EntityManager::clearAddedComponents(ComponentTypeId type)
{
    std::vector<EntityId> garbage;
    componentsAdded[type].swap(garbage);
    bin.push_back(std::move(garbage));
}

void
EntityManager::clearRemovedComponents(ComponentTypeId type)
{
    std::vector<EntityId> garbage;
    componentsRemoved[type].swap(garbage);
    bin.push_back(std::move(garbage));
}

void
EntityManager::removeEntity(EntityId entity) noexcept
{
    auto archetype = entityArchetypes[entity];
    ArchetypeManager::removeEntity(archetype, entity);

    auto archetypeSignature = ArchetypeManager::getArchetypeSignature(archetype);
    archetypeSignature.iterateNonZeroBits(
        [&entity](std::uint64_t componentTypeId)
        {
            componentsRemoved[componentTypeId].push_back(entity);
            removedComponentsVersion[componentTypeId] = Util::Clock::getInstance().count;
        });

    entityAllocator.returnName(entity);
}

void
EntityManager::updateAddedComponents(ComponentTypeId type)
{
    if (addedComponentsVersion[type] < Util::Clock::getInstance().count)
        clearAddedComponents(type);
}

void
EntityManager::updateRemovedComponents(ComponentTypeId type)
{
    if (removedComponentsVersion[type] < Util::Clock::getInstance().count)
        clearRemovedComponents(type);
}

Generic::Util::NameAllocator              EntityManager::entityAllocator;
std::unordered_map<EntityId, ArchetypeId> EntityManager::entityArchetypes;
std::uint64_t                             structuralChangesVersion = 0;
} // namespace Generic::Core::ECS