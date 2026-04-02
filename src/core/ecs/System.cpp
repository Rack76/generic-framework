#include "core/ecs/System.h"

namespace Trimego::Core::ECS
{
void
System::addReadBatch(EntityReadProc                         entityReadProc,
                     std::initializer_list<ComponentTypeId> include,
                     std::initializer_list<ComponentTypeId> changed,
                     std::initializer_list<ComponentTypeId> exclude)
{
    batches.push_back(
        [entityReadProc, include, exclude, changed]()
        {
            ArchetypeIdBitset archetypes = ArchetypeManager::readArchetypesWith(include, exclude);
            archetypes.iterateNonZeroBits(
                [&changed, &entityReadProc](std::uint64_t archetype)
                {
                    EntityBitset entityBitset =
                        ArchetypeManager::getEntityBitset(archetype, changed);
                    auto& globalEntityIds = ArchetypeManager::getGlobalEntityIds(archetype);
                    entityBitset.iterateNonZeroBits(
                        [&entityReadProc, &globalEntityIds](std::uint64_t entity)
                        { entityReadProc(globalEntityIds.at(entity)); });
                });
        });
}

void
System::addWriteBatch(EntityReadProc                         entityWriteProc,
                      std::initializer_list<ComponentTypeId> include,
                      std::initializer_list<ComponentTypeId> changed,
                      std::initializer_list<ComponentTypeId> exclude)
{
    batches.push_back(
        [entityWriteProc, include, exclude, changed]()
        {
            ArchetypeIdBitset archetypes = ArchetypeManager::writeArchetypesWith(include, exclude);
            archetypes.iterateNonZeroBits(
                [&changed, &entityWriteProc](std::uint64_t archetype)
                {
                    EntityBitset entityBitset =
                        ArchetypeManager::getEntityBitset(archetype, changed);
                    auto& globalEntityIds = ArchetypeManager::getGlobalEntityIds(archetype);
                    entityBitset.iterateNonZeroBits(
                        [&entityWriteProc, &globalEntityIds](std::uint64_t entity)
                        { entityWriteProc(globalEntityIds.at(entity)); });
                });
        });
}

void
System::addOnComponentAdded(EntityProc entityProc, ComponentTypeId type)
{
    onComponentAddedCallbacks.push_back(
        [type, entityProc]()
        {
            EntityManager::updateAddedComponents(type);
            auto& entities = EntityManager::getAddedComponents(type);
            for (auto entity : entities)
            {
                entityProc(entity);
            }
        });
}

void
System::addOnComponentRemoved(EntityProc entityProc, ComponentTypeId type)
{
    onComponentRemovedCallbacks.push_back(
        [type, entityProc]()
        {
            EntityManager::updateRemovedComponents(type);
            auto& entities = EntityManager::getRemovedComponents(type);
            for (auto entity : entities)
            {
                entityProc(entity);
            }
        });
}

void
System::run() noexcept
{
    for (auto& callback : onComponentAddedCallbacks)
    {
        callback();
    }
    for (auto& callback : onComponentRemovedCallbacks)
    {
        callback();
    }
    for (auto& batch : batches)
    {
        batch();
    }
}
} // namespace Trimego::Core::ECS
