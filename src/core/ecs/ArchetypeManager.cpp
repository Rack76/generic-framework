#include "core/ecs/ArchetypeManager.h"

namespace Generic::Core::ECS
{
ArchetypeId
ArchetypeManager::addArchetype(ArchetypeSignature& archetypeSignature)
{
    std::uint64_t archetypeId = archetypeCount++;
    archetypes.insert({archetypeId, Archetype(50, archetypeSignature)});

    archetypeSignature.iterateNonZeroBits(
        [&archetypeId](std::uint64_t componentTypeId)
        { archetypeInverseIndex[componentTypeId].setBit(archetypeId); });

    archetypeIds[archetypeSignature] = archetypeId;

    return archetypeId;
}

ArchetypeId
ArchetypeManager::getArchetypeId(ArchetypeSignature&& archetypeSignature)
{
    ArchetypeId archetypeId;
    if (archetypeIds.find(archetypeSignature) == archetypeIds.end())
        archetypeId = addArchetype(archetypeSignature);
    else
        archetypeId = archetypeIds[archetypeSignature];
    return archetypeId;
}

ArchetypeId
ArchetypeManager::getArchetypeId(ArchetypeSignature& archetypeSignature)
{
    ArchetypeId archetypeId;
    if (archetypeIds.find(archetypeSignature) == archetypeIds.end())
        archetypeId = addArchetype(archetypeSignature);
    else
        archetypeId = archetypeIds[archetypeSignature];
    return archetypeId;
}

void
ArchetypeManager::transferEntityToSuperArchetype(
    EntityId                               entity,
    ArchetypeId                            dest,
    ArchetypeId                            source,
    std::initializer_list<ComponentTypeId> additionalComponents)
{
    auto components = archetypes[source].getComponents(entity);
    archetypes[dest].addEntity(entity, components);
    archetypes[dest].setComponents(entity, additionalComponents);
    archetypes[source].removeEntity(entity);
}

void
ArchetypeManager::transferEntityToSuperArchetype(
    EntityId                                        entity,
    ArchetypeId                                     dest,
    ArchetypeId                                     source,
    std::vector<std::pair<ComponentTypeId, void*>>& additionalComponents)
{
    auto components = archetypes[source].getComponents(entity);
    archetypes[dest].addEntity(entity, components);
    archetypes[dest].setComponents(entity, additionalComponents);
    archetypes[source].removeEntity(entity);
}

void
ArchetypeManager::transferEntityToSubArchetype(EntityId                      entity,
                                               ArchetypeId                   dest,
                                               ArchetypeId                   source,
                                               std::vector<ComponentTypeId>& signature)
{
    auto remainingComponents = archetypes[source].getComponents(entity, signature);
    archetypes[dest].addEntity(entity, remainingComponents);
    archetypes[source].removeEntity(entity);
}

void
ArchetypeManager::addEntity(ArchetypeId archetypeId, EntityId entity)
{
    archetypes.at(archetypeId).addEntity(entity);
}

void
ArchetypeManager::addEntity(ArchetypeId                                     archetypeId,
                            EntityId                                        entity,
                            std::vector<std::pair<ComponentTypeId, void*>>& values)
{
    archetypes.at(archetypeId).addEntity(entity, values);
}

const void*
ArchetypeManager::readComponent(EntityId        entity,
                                ComponentTypeId type,
                                ArchetypeId     archetypeId) noexcept
{
    return archetypes[archetypeId].readComponent(entity, type);
}

void*
ArchetypeManager::writeComponent(EntityId        entity,
                                 ComponentTypeId type,
                                 ArchetypeId     archetypeId) noexcept
{
    return archetypes[archetypeId].writeComponent(entity, type);
}

void
ArchetypeManager::removeEntity(ArchetypeId archetypeId, EntityId entity) noexcept
{
    archetypes.at(archetypeId).removeEntity(entity);
}

ArchetypeIdBitset
ArchetypeManager::readArchetypesWith(std::initializer_list<ComponentTypeId> include,
                                     std::initializer_list<ComponentTypeId> exclude)
{
    ArchetypeIdBitset archetypes;
    auto              it = include.begin();
    archetypes           = archetypeInverseIndex[*it].OR(archetypes);

    for (it++; it != include.end(); it++)
    {
        archetypes = archetypes.AND(archetypeInverseIndex[*it]);
    }
    for (auto componentTypeId : exclude)
    {
        archetypes = archetypes.excludeBits(archetypeInverseIndex[componentTypeId]);
    }
    return archetypes;
}

ArchetypeIdBitset
ArchetypeManager::writeArchetypesWith(std::initializer_list<ComponentTypeId> include,
                                      std::initializer_list<ComponentTypeId> exclude)
{
    ArchetypeIdBitset _archetypes;
    auto              it = include.begin();
    _archetypes          = archetypeInverseIndex[*it].OR(_archetypes);

    for (it++; it != include.end();)
    {
        _archetypes = _archetypes.AND(archetypeInverseIndex[*it]);
    }
    for (auto componentTypeId : exclude)
    {
        _archetypes = _archetypes.excludeBits(archetypeInverseIndex[componentTypeId]);
    }

    _archetypes.iterateNonZeroBits(
        [](std::uint64_t archetypeId)
        {
            auto& archetype = archetypes[archetypeId];
            if (archetype.getCurrentDataChangesVersion() < Util::Clock::getInstance().count)
            {
                archetypes[archetypeId].updateDataChangesVersion();
                archetypes[archetypeId].clear();
            }
        });

    return _archetypes;
}

EntityBitset
ArchetypeManager::getEntityBitset(ArchetypeId                            archetype,
                                  std::initializer_list<ComponentTypeId> changed)
{
    return archetypes[archetype].getEntityBitset(changed);
}

const std::vector<EntityId>&
ArchetypeManager::getGlobalEntityIds(ArchetypeId archetype)
{
    return archetypes[archetype].getGlobalIds();
}

bool
ArchetypeManager::archetypeExists(ArchetypeSignature& archetypeSignature) noexcept
{
    return archetypeIds.find(archetypeSignature) != archetypeIds.end();
}

const ArchetypeSignature
ArchetypeManager::getArchetypeSignature(ArchetypeId archetype)
{
    return archetypes[archetype].getSignature();
}

std::unordered_map<ComponentTypeId, ArchetypeIdBitset> ArchetypeManager::archetypeInverseIndex;
std::unordered_map<EntityId, ArchetypeId>              ArchetypeManager::entityArchetypes;
std::unordered_map<std::uint64_t, Archetype>           ArchetypeManager::archetypes;
std::unordered_map<ArchetypeSignature, std::uint64_t>  ArchetypeManager::archetypeIds;
std::uint64_t                                          ArchetypeManager::archetypeCount;
} // namespace Generic::Core::ECS