#ifndef _TRIMEGO_ENTITY_MANAGER_H_
#define _TRIMEGO_ENTITY_MANAGER_H_

#include <cstdint>
#include "ArchetypeManager.h"
#include "util/CTTI.h"
#include "util/NameAllocator.h"

using EntityId = std::uint64_t;

namespace Trimego::Core::ECS
{
class EntityManager
{
public:
    /**
     * @brief adds an entity.
     * @param Types component types to initalize the entity with.
     * @return a handle to the added entity.
     */
    template <typename... Types>
    [[nodiscard]] static EntityId addEntity()
    {
        static std::uint64_t archetypeId =
            ArchetypeManager::getArchetypeId(ArchetypeSignature({Util::typeId<Types>()...}));

        EntityId entity          = entityAllocator.getName();
        entityArchetypes[entity] = archetypeId;
        ArchetypeManager::addEntity(archetypeId, entity);

        auto componentTypeIds = {Util::typeId<Types>()...};
        for (auto componentTypeId : componentTypeIds)
        {
            componentsAdded[componentTypeId].push_back(entity);
            addedComponentsVersion[componentTypeId] = Util::Clock::getInstance().count;
        }

        return entity;
    }

    /**
     * @brief adds an entity, allows its components to be initialized with chosen values.
     * @param components components to initalize the entity with.
     * @return a handle to the added entity.
     */
    template <typename... Types>
    [[nodiscard]] static EntityId addEntity(Types&&... components)
    {
        static std::uint64_t archetypeId =
            ArchetypeManager::getArchetypeId(ArchetypeSignature({Util::typeId<Types>()...}));

        EntityId entity          = entityAllocator.getName();
        entityArchetypes[entity] = archetypeId;
        ArchetypeManager::addEntity(
            archetypeId, entity, {{Util::typeId<Types>(), static_cast<void*>(&components)}...});

        auto componentTypeIds = {Util::typeId<Types>()...};
        for (auto componentTypeId : componentTypeIds)
        {
            componentsAdded[componentTypeId].push_back(entity);
            addedComponentsVersion[componentTypeId] = Util::Clock::getInstance().count;
        }

        return entity;
    }

    /**
     * @brief adds components to an entity.
     * @param Types component types to add to the entity.
     * @param entity the entity to add components to.
     */
    template <typename... Types>
    static void addComponents(EntityId entity)
    {
        ArchetypeSignature sourceArchetypeSignature =
            ArchetypeManager::getArchetypeSignature(entityArchetypes[entity]);
        ArchetypeSignature additionalSignature = ArchetypeSignature({Util::typeId<Types>()...});
        ArchetypeSignature destArchetypeSignature =
            sourceArchetypeSignature.OR(additionalSignature);

        ArchetypeId sourceArchetypeId = entityArchetypes[entity];
        ArchetypeId destArchetypeId   = ArchetypeManager::getArchetypeId(destArchetypeSignature);

        if (sourceArchetypeId == destArchetypeId)
            return; //

        ArchetypeManager::transferEntityToSuperArchetype(
            entity, destArchetypeId, sourceArchetypeId, {Util::typeId<Types>()...});
        entityArchetypes[entity] = destArchetypeId;

        auto componentTypeIds = {Util::typeId<Types>()...};
        for (auto componentTypeId : componentTypeIds)
        {
            componentsAdded[componentTypeId].push_back(entity);
            addedComponentsVersion[componentTypeId] = Util::Clock::getInstance().count;
        }
    }

    /**
     * @brief adds components to an entity, allows its components to be initialized with chosen
     * values.
     * @param values components to add to the entity.
     * @param entity the entity to add components to.
     */
    template <typename... Types>
    static void addComponents(EntityId entity, Types&&... values)
    {
        ArchetypeSignature sourceArchetypeSignature =
            ArchetypeManager::getArchetypeSignature(entityArchetypes[entity]);
        ArchetypeSignature additionalSignature = ArchetypeSignature({Util::typeId<Types>()...});
        ArchetypeSignature destArchetypeSignature =
            sourceArchetypeSignature.OR(additionalSignature);

        ArchetypeId sourceArchetypeId = entityArchetypes[entity];
        ArchetypeId destArchetypeId   = ArchetypeManager::getArchetypeId(destArchetypeSignature);

        if (sourceArchetypeId == destArchetypeId)
            return; //

        ArchetypeManager::transferEntityToSuperArchetype(
            entity,
            destArchetypeId,
            sourceArchetypeId,
            {{Util::typeId<Types>(), static_cast<void*>(&values)}...});
        entityArchetypes[entity] = destArchetypeId;

        auto componentTypeIds = {Util::typeId<Types>()...};
        for (auto componentTypeId : componentTypeIds)
        {
            componentsAdded[componentTypeId].push_back(entity);
            addedComponentsVersion[componentTypeId] = Util::Clock::getInstance().count;
        }
    }

    /**
     * @brief adds components to an entity.
     * @param Types component types to remove from the entity.
     * @param entity the entity to remove components from.
     */
    template <typename... Types>
    static void removeComponents(EntityId entity) noexcept
    {
        ArchetypeSignature sourceArchetypeSignature =
            ArchetypeManager::getArchetypeSignature(entityArchetypes[entity]);
        ArchetypeSignature additionalSignature = ArchetypeSignature({Util::typeId<Types>()...});
        ArchetypeSignature destArchetypeSignature =
            sourceArchetypeSignature.excludeBits(additionalSignature);

        ArchetypeId sourceArchetypeId = entityArchetypes[entity];
        ArchetypeId destArchetypeId   = ArchetypeManager::getArchetypeId(destArchetypeSignature);

        if (sourceArchetypeId == destArchetypeId)
            return; //

        ArchetypeManager::transferEntityToSubArchetype(
            entity, destArchetypeId, sourceArchetypeId, {Util::typeId<Types>()...});
        entityArchetypes[entity] = destArchetypeId;

        auto componentTypeIds = {Util::typeId<Types>()...};
        for (auto componentTypeId : componentTypeIds)
        {
            componentsRemoved[componentTypeId].push_back(entity);
            removedComponentsVersion[componentTypeId] = Util::Clock::getInstance().count;
        }
    }

    /**
     * @brief retrives a pointer to a component.
     * @param entity the entity to retrive the component from.
     * @param T the type of the component to retrieve.
     * @return a pointer to the component which is immutable.
     */
    template <typename T>
    [[nodiscard]] static const T* readComponent(EntityId entity) noexcept
    {
        ArchetypeId archetype = entityArchetypes[entity];
        return reinterpret_cast<const T*>(
            ArchetypeManager::readComponent(entity, Util::typeId<T>(), archetype));
    }

    /**
     * @brief retrives a pointer to a component.
     * @param entity the entity to retrive the component from.
     * @param T the type of the component to retrieve.
     * @return a pointer to the component which is mutable.
     */
    template <typename T>
    [[nodiscard]] static T* writeComponent(EntityId entity) noexcept
    {
        ArchetypeId archetype = entityArchetypes[entity];
        return reinterpret_cast<T*>(
            ArchetypeManager::writeComponent(entity, Util::typeId<T>(), archetype));
    }

    /**
     * @brief retrives entities whose components were added since the beginning of the current
     * frame.
     * @param type the specific type id of the components of the entities to retrive.
     * @return a reference to a vector containing the entities whose components were added.
     */
    static std::vector<EntityId>& getAddedComponents(ComponentTypeId type);

    /**
     * @brief retrives entities whose components were removed since the beginning of the current
     * frame.
     * @param type the specific type id of the components of the entities to retrive.
     * @return a reference to a vector containing the entities whose components were removed.
     */
    static std::vector<EntityId>& getRemovedComponents(ComponentTypeId type);

    /**
     * @brief removes an entity.
     * @param entityId the entity to remove.
     */
    static void removeEntity(EntityId entityId) noexcept;

    /**
     * @brief checks the version of the current added components array, if it is less than the
     * current frame, clears this array.
     * @note this method is used in the context of reactive systems.
     */
    static void updateAddedComponents(ComponentTypeId type);

    /**
     * @brief checks the version of the current removed components array, if it is less than the
     * current frame, clears this array.
     * @note this method is used in the context of reactive systems.
     */
    static void updateRemovedComponents(ComponentTypeId type);

private:
    static void clearAddedComponents(ComponentTypeId type);

    static void clearRemovedComponents(ComponentTypeId type);

    static Trimego::Util::NameAllocator                               entityAllocator;
    static std::unordered_map<EntityId, ArchetypeId>                  entityArchetypes;
    static std::unordered_map<ComponentTypeId, std::uint64_t>         addedComponentsVersion;
    static std::unordered_map<ComponentTypeId, std::uint64_t>         removedComponentsVersion;
    static std::unordered_map<ComponentTypeId, std::vector<EntityId>> componentsAdded;
    static std::unordered_map<ComponentTypeId, std::vector<EntityId>> componentsRemoved;
    static std::vector<std::vector<EntityId>>                         bin;
};
} // namespace Trimego::Core::ECS

#endif