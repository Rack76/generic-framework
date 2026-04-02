#include "core/ecs/Archetype.h"

namespace Trimego::Core::ECS
{
Archetype::Archetype(std::uint64_t entityCapacity, ArchetypeSignature& signature)
    : signature(signature)
{
    signature.iterateNonZeroBits([this, &entityCapacity](std::uint64_t componentTypeId)
                                 { entities[componentTypeId] = new void*[entityCapacity]; });
    setBlockHandles(signature);
}

Archetype::Archetype(Archetype&& source)
{
    blockHandles = std::move(source.blockHandles);
    entities     = std::move(source.entities);
    signature    = std::move(source.signature);
    source.entities.clear();
}

void
Archetype::addEntity(EntityId entity)
{
    entityCount++;

    EntityLocalId localId = localIds[entity] = localEntityAllocator.getName();
    globalIds[localId]                       = entity;

    for (auto pair : entities)
    {
        entities[pair.first][localId] =
            ComponentManager::getComponent(blockHandles[pair.first], pair.first);
    }

    entityBitset.setBit(localId);
}

void
Archetype::addEntity(EntityId entity, std::vector<std::pair<ComponentTypeId, void*>>& values)
{
    entityCount++;

    EntityLocalId localId = localIds[entity] = localEntityAllocator.getName();
    globalIds[localId]                       = entity;

    for (auto pair : values)
    {
        auto componentTypeId               = pair.first;
        auto component                     = pair.second;
        entities[componentTypeId][localId] = ComponentManager::getComponent(
            blockHandles[componentTypeId], componentTypeId, component);
    }

    entityBitset.setBit(localId);
}

std::vector<std::pair<ComponentTypeId, void*>>
Archetype::getComponents(EntityId entity) noexcept
{
    std::vector<std::pair<ComponentTypeId, void*>> components;

    if (localIds.find(entity) == localIds.end())
        return {};

    EntityLocalId localId = localIds[entity];

    for (const auto& pair : entities)
    {
        ComponentTypeId componentTypeId = pair.first;
        components.push_back({componentTypeId, entities[componentTypeId][localId]});
    }

    return components;
}

std::vector<std::pair<ComponentTypeId, void*>>
Archetype::getComponents(EntityId entity, std::vector<ComponentTypeId>& signature) noexcept
{
    std::vector<std::pair<ComponentTypeId, void*>> components;

    if (localIds.find(entity) == localIds.end())
        return {}; //

    EntityLocalId localId = localIds[entity];

    for (const auto& componentTypeId : signature)
    {
        components.push_back({componentTypeId, entities[componentTypeId][localId]});
    }

    return components;
}

void
Archetype::setComponents(EntityId                               entity,
                         std::initializer_list<ComponentTypeId> componentTypeIds) noexcept
{
    if (localIds.find(entity) == localIds.end())
        return;

    EntityLocalId localId = localIds[entity];

    for (auto componentTypeId : componentTypeIds)
    {
        entities[componentTypeId][localId] =
            ComponentManager::getComponent(blockHandles[componentTypeId], componentTypeId);
    }
}

void
Archetype::setComponents(EntityId                                        entity,
                         std::vector<std::pair<ComponentTypeId, void*>>& components) noexcept
{
    if (localIds.find(entity) == localIds.end())
        return;

    EntityLocalId localId = localIds[entity];

    for (auto pair : components)
    {
        auto  componentTypeId = pair.first;
        void* value           = pair.second;
        entities[componentTypeId][localId] =
            ComponentManager::getComponent(blockHandles[componentTypeId], componentTypeId, value);
    }
}

void
Archetype::removeEntity(EntityId entity) noexcept
{
    int lastLocalEntity = localEntityAllocator.lastNameReserved();
    if (entityCount == 0)
        return;

    entityCount--;

    if (localIds.find(entity) == localIds.end())
        return;

    EntityLocalId localId = localIds[entity];

    for (auto pair : entities)
    {
        auto  componentTypeId = pair.first;
        void* source          = entities[componentTypeId][lastLocalEntity];
        auto  blockHandle     = blockHandles[componentTypeId];

        void* dest = entities[componentTypeId][localId];
        ComponentManager::copyComponent(blockHandle, componentTypeId, dest, source);
        ComponentManager::returnComponent(componentTypeId, blockHandle, source);
    }

    auto globalId      = globalIds[lastLocalEntity];
    localIds[globalId] = localId;
    globalIds[localId] = globalId;

    localIds.erase(entity);

    localEntityAllocator.returnName(lastLocalEntity);

    entityBitset.unsetBit(lastLocalEntity);
}

EntityBitset
Archetype::getEntityBitset(std::initializer_list<ComponentTypeId> changed)
{
    if (changed.size() >= 1)
    {
        EntityBitset _entityBitset = entityBitset.AND(entitiesChanged.begin()->second);
        auto         it            = changed.begin();
        for (it++; it != changed.end(); it++)
        {
            _entityBitset = _entityBitset.AND(entitiesChanged[*it]);
        }
        return _entityBitset;
    }
    else
        return entityBitset;
}

void
Archetype::setBlockHandles(ArchetypeSignature& signature)
{
    signature.iterateNonZeroBits(
        [this](std::uint64_t componentTypeId)
        { blockHandles[componentTypeId] = ComponentManager::getBlockHandle(componentTypeId); });
}

Archetype::~Archetype() noexcept
{
    for (auto pair : entities)
    {
        if (pair.second != nullptr)
            delete[] pair.second;
    }
}

const void*
Archetype::readComponent(EntityId entity, ComponentTypeId type) noexcept
{
#ifndef NDEBUG
    if (localIds.find(entity) == localIds.end())
        return nullptr; //
    if (entities.find(type) == entities.end())
        return nullptr;
#endif

    auto localId = localIds[entity];
    return entities[type][localId];
}

void*
Archetype::writeComponent(EntityId entity, ComponentTypeId type) noexcept
{
#ifndef NDEBUG
    if (localIds.find(entity) == localIds.end())
        return nullptr; //
    if (entities.find(type) == entities.end())
        return nullptr;
#endif

    entitiesChanged[type].setBit(localIds[entity]);

    auto localId = localIds[entity];
    return entities[type][localId];
}
} // namespace Trimego::Core::ECS
