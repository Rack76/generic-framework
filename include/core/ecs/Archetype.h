#ifndef _TRIMEGO_ARCHETYPE_H_
#define _TRIMEGO_ARCHETYPE_H_

#include <cstdint>
#include <unordered_map>
#include <vector>
#include "core/Clock.h"
#include "core/ecs/ComponentManager.h"
#include "util/NameAllocator.h"
#include "util/VLUI.hpp"

namespace Trimego::Core::ECS
{
using ArchetypeSignature = Trimego::Util::VLUI;
using EntityBitset       = Trimego::Util::VLUI;
using EntityId           = std::uint64_t;
using EntityLocalId      = std::uint64_t;
using ComponentTypeId    = std::uint64_t;

class Archetype final
{
public:
    Archetype()
    {
    }

    /**
     * @brief constructs an archetype
     * @param entityCapacity maximum number of entities the archetype can hold
     * @param signature bitset representing the component types of the archetype
     */
    Archetype(std::uint64_t entityCapacity, ArchetypeSignature& signature);

    Archetype(Archetype&& source);

    ~Archetype() noexcept;

    /**
     * @brief Retrieves a read only pointer to a component of a specific type.
     * @param entity The global ID of the entity to query.
     * @param type The ID of the component type.
     * @return A const pointer to the component, or nullptr if not found.
     * @note nullptr is returned only in debug mode
     */
    [[nodiscard]] const void* readComponent(EntityId entity, ComponentTypeId type) noexcept;

    /**
     * @brief Retrieves a mutable pointer to a component of a specific type.
     * @param entity The global ID of the entity to query.
     * @param type The ID of the component type.
     * @return A pointer to the component, or nullptr if not found.
     * @note nullptr is returned only in debug mode
     */
    void* writeComponent(EntityId entity, ComponentTypeId type) noexcept;

    /**
     * @brief Registers an entity within the archetype.
     * @details Maps the entity's global ID to its internal local ID.
     * @param entity The global identifier of the entity to add.
     */
    void addEntity(EntityId entity);

    /**
     * @brief Registers an entity within the archetype and initialises it.
     * @details Maps the entity's global ID to its internal local ID.
     * @param entity The global identifier of the entity to add.
     * @param values the values to initialize the entity with.
     */
    void addEntity(EntityId entity, std::vector<std::pair<ComponentTypeId, void*>>& values);

    /**
     * @brief Registers an entity within the archetype and initialises some of its components.
     * @details Maps the entity's global ID to its internal local ID.
     * @param entity The global identifier of the entity to add.
     * @param values the values to initialize the entity with.
     * @param signature bitset representing the component types to initialise the entity with.
     * @note this method is specifically used to move an entity between two archetypes in the
     * context of components addition or removal.
     */
    void addEntity(EntityId                                        entity,
                   ArchetypeSignature&                             signature,
                   std::vector<std::pair<ComponentTypeId, void*>>& values);

    /**
     * @brief retrives an entity's components.
     * @param entity the global ID of the entity to retrive the components from.
     * @return a hash map of component type id / component pointer pair.
     * @note this method is used to retrive components in the context of archetype migration.
     * */
    std::vector<std::pair<ComponentTypeId, void*>> getComponents(EntityId entity) noexcept;

    /**
     * @brief retrives an entity's components.
     * @param entity the global ID of the entity to retrive the components from.
     * @param signature a bitmask of component type ids representing the specific component types to
     * fetch.
     * @return a hash map of component type id / component pointer pair.
     * @note this method is used to retrive components in the context of archetype migration.
     * */
    std::vector<std::pair<ComponentTypeId, void*>>
    getComponents(EntityId entity, std::vector<ComponentTypeId>& signature) noexcept;

    /**
     * @brief binds an entity component pointers to their source pools.
     * @details Fetches and assigns the memory addresses for specific component types
     * from the global object pools to the entity's new slot in the archetype.
     * @param entity The global identifier of the entity.
     * @param componentTypeIds A list of component types to be linked from the pools.
     * @note This is the final step of an archetype migration to ensure all
     * non-transferred components are correctly pointed to.
     */
    void setComponents(EntityId                               entity,
                       std::initializer_list<ComponentTypeId> componentTypeIds) noexcept;

    /**
     * @brief binds an entity component pointers to their source pools and initializes their value.
     * @details Fetches and assigns the memory addresses for specific component types
     * from the global object pools to the entity's new slot in the archetype.
     * @param entity The global identifier of the entity.
     * @param components a hash map of component type ids / component pointer pairs.
     * @note This is the final step of an archetype migration to ensure all
     * non-transferred components are correctly pointed to.
     */
    void setComponents(EntityId                                        entity,
                       std::vector<std::pair<ComponentTypeId, void*>>& components) noexcept;

    /**
     * @brief removes an entity from the archetype.
     * @param entity The global identifier of the entity.
     * @details the entity to be removed is
     * replaced by the last entity in the archetype to maintain memory contiguity.
     * @note This ensures that component data remains packed and cache-friendly
     * by avoiding fragmentation (holes) within the internal storage.
     */
    void removeEntity(EntityId entity) noexcept;

    /**
     * @brief Retrieves a bitset of local entity identifiers, optionally filtered by change status.
     * @details If the provided list is empty, returns all active entities within the archetype.
     * Otherwise, filters and returns only those entities whose specified component types
     * have been modified since the start of the current frame.
     * @param changed A list of component type IDs to check for modifications (dirty flags).
     * @return An EntityBitset representing the qualifying local entities.
     * @note This method is essential for **reactive systems** that only process
     * entities with "dirty" or updated component data.
     */
    [[nodiscard]] EntityBitset getEntityBitset(std::initializer_list<ComponentTypeId> changed);

    /**
     * @brief Provides access to the local-to-global entity ID mapping.
     * @details Returns a reference to the internal map used to resolve a local
     * storage index (EntityLocalId) back to its unique global identifier (EntityId).
     * @return A const reference to the entity ID resolution map.
     * @note This is typically used by systems to identify entities when
     * iterating over the archetype's contiguous storage.
     */
    const std::vector<EntityId>& getGlobalIds()
    {
        return globalIds;
    }

    /**
     * @brief Returns the archetype's current data changes version, representing the last frame its
     * entities were modified.
     * @details This version acts as a timestamp to track data changes for reactive systems.
     * @return The current version as a 64-bit unsigned integer.
     */
    const std::uint64_t getCurrentDataChangesVersion()
    {
        return dataChangesVersion;
    }

    /**
     * @brief Updates the archetype's current data changes version, representing the last frame its
     * entities were modified.
     * @details This version acts as a timestamp to track data changes for reactive systems.
     */
    inline void updateDataChangesVersion() noexcept
    {
        dataChangesVersion = Util::Clock::getInstance().count;
    }

    /**
     * @brief clears the list of all entities that changed.
     * @note this method resets all bitsets containing changed entities
     * and is used to implement reactive systems.
     */
    inline void clear() noexcept
    {
        for (auto& pair : entitiesChanged)
        {
            auto& entityBitset = pair.second;
            entityBitset.clear();
        }
    }

    /**
     * @brief Returns the archetype signature which is
     * a bitmask representing the archetype component types.
     * @return the archetype signature as a VLUI.
     */
    [[nodiscard]] inline const ArchetypeSignature& getSignature() noexcept
    {
        return signature;
    }

private:
    void                                             setBlockHandles(ArchetypeSignature& signature);
    std::unordered_map<EntityId, EntityLocalId>      localIds;
    std::vector<EntityId>                            globalIds;
    std::unordered_map<std::uint64_t, std::uint64_t> blockHandles;
    std::unordered_map<ComponentTypeId, void**>      entities;
    std::unordered_map<ComponentTypeId, EntityBitset> entitiesChanged;
    Trimego::Util::NameAllocator                      localEntityAllocator;
    ArchetypeSignature                                signature;
    EntityBitset                                      entityBitset;
    std::uint64_t                                     entityCount        = 0;
    std::uint64_t                                     dataChangesVersion = 0;
};
} // namespace Trimego::Core::ECS

#endif