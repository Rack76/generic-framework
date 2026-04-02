#ifndef _GENERIC_ARCHETYPE_MANAGER_H_
#define _GENERIC_ARCHETYPE_MANAGER_H_

#include <cstdint>
#include <initializer_list>
#include <unordered_map>
#include "Archetype.h"

using EntityId          = std::uint64_t;
using ArchetypeId       = std::uint64_t;
using ArchetypeIdBitset = Generic::Util::VLUI;

namespace Generic::Core::ECS
{
class ArchetypeManager
{
public:
    /**
     * @brief Adds a new archetype based on the provided signature and returns a handle to it.
     * @param archetypeSignature a bitmask representing the archetype
     * signature which is a bitset of component type ids.
     * @return the archetype id as an std::uint64_t.
     */
    [[nodiscard]] static ArchetypeId addArchetype(ArchetypeSignature& archetypeSignature);

    /**
     * @brief returns a handle to the archetype specified by the the provided signature.
     * @param archetypeSignature a bitmask representing the archetype
     * signature which is a bitset of component type ids.
     * @return the archetype id as an std::uint64_t.
     * @note if the archetype doesnt exists,
     * this method will create it based on the provided signature and return a handle to it.
     */
    [[nodiscard]] static ArchetypeId getArchetypeId(ArchetypeSignature&& archetypeSignature);

    /**
     * @brief returns a handle to the archetype specified by the the provided signature.
     * @param archetypeSignature a bitmask representing the archetype
     * signature which is a bitset of component type ids.
     * @return the archetype id as an std::uint64_t.
     * @note if the archetype doesnt exists,
     * this method will create it based on the provided signature and return a handle to it.
     */
    [[nodiscard]] static ArchetypeId getArchetypeId(ArchetypeSignature& archetypeSignature);

    /**
     * @brief transfers an entity to a superarchetype of its current archetype.
     * @param entity the global id of the entity to transfer.
     * @param dest the id of the destination archetype.
     * @param source the id of the source archetype.
     * @param additionalComponents the ids of the component types missing in the current archetype.
     */
    static void
    transferEntityToSuperArchetype(EntityId                               entity,
                                   ArchetypeId                            dest,
                                   ArchetypeId                            source,
                                   std::initializer_list<ComponentTypeId> additionalComponents);

    /**
     * @brief transfers an entity to a superarchetype of its current archetype.
     * @param entity the global id of the entity to transfer.
     * @param dest the id of the destination archetype.
     * @param source the id of the source archetype.
     * @param additionalComponents a vector of pairs containing the IDs of the additional
     * component types and their respective components.
     */
    static void transferEntityToSuperArchetype(
        EntityId                                        entity,
        ArchetypeId                                     dest,
        ArchetypeId                                     source,
        std::vector<std::pair<ComponentTypeId, void*>>& additionalComponents);

    /**
     * @brief transfers an entity to a subarchetype of its current archetype.
     * @param entity the global id of the entity to transfer.
     * @param dest the id of the destination archetype.
     * @param source the id of the source archetype.
     * @param additionalComponents the ids of the subarchetype's component types.
     */
    static void transferEntityToSubArchetype(EntityId                      entity,
                                             ArchetypeId                   dest,
                                             ArchetypeId                   source,
                                             std::vector<ComponentTypeId>& remainingComponents);

    /**
     * @brief adds an entity to an archetype
     * @param entity the global id of the entity to add.
     * @param archetypeId the id of the archetype to which add the entity.
     */
    static void addEntity(ArchetypeId archetypeId, EntityId entity);

    /**
     * @brief adds an entity to an archetype and initializes it.
     * @param entity the global id of the entity to add.
     * @param archetypeId the id of the archetype to which add the entity.
     * @param values the values to initialize the entity with.
     */
    static void addEntity(ArchetypeId                                     archetypeId,
                          EntityId                                        entity,
                          std::vector<std::pair<ComponentTypeId, void*>>& values);

    /**
     * @brief retrives a pointer to a component.
     * @param entity the entity to retrive the component from.
     * @param type the type id of the component.
     * @param archetypeId the id of archetype owing the component.
     * @return a pointer to the component which is immutable.
     */
    [[nodiscard]] static const void*
    readComponent(EntityId entity, ComponentTypeId type, ArchetypeId archetypeId) noexcept;

    /**
     * @brief retrives a pointer to a component.
     * @param entity the entity to retrive the component from.
     * @param type the type id of the component.
     * @param archetypeId the id of archetype owing the component.
     * @return a pointer to the component which is mutable.
     */
    [[nodiscard]] static void*
    writeComponent(EntityId entity, ComponentTypeId type, ArchetypeId archetypeId) noexcept;

    /**
     * @brief removes an entity from an archetype.
     * @param entity the global id of the entity to remove.
     * @param archetypeId the id of the archetype to remove the entity from.
     */
    static void removeEntity(ArchetypeId archetypeId, EntityId entity) noexcept;

    /**
     * @brief returns handles to archetypes that include and/or exclude specific component types.
     * @param include the type ids of the component to include.
     * @param exclude the type ids of the component to exclude.
     * @return the list of archetypes as a variable length bitset.
     * @note use it when you want to read from archetypes.
     */
    [[nodiscard]] static ArchetypeIdBitset
    readArchetypesWith(std::initializer_list<ComponentTypeId> include,
                       std::initializer_list<ComponentTypeId> exclude = {});

    /**
     * @brief returns handles to archetypes that include and/or exclude specific component types.
     * @param include the type ids of the component to include.
     * @param exclude the type ids of the component to exclude.
     * @return the list of archetypes as a variable length bitset.
     * @note use it when you want to write to archetypes.
     */
    [[nodiscard]] static ArchetypeIdBitset
    writeArchetypesWith(std::initializer_list<ComponentTypeId> include,
                        std::initializer_list<ComponentTypeId> exclude = {});

    /**
     * @brief returns a list of local handles to entities inside an archetype.
     * @param archetype the archetype id to read the local handles from.
     * @param changed the list of type ids of the components that changed since the beginning of the
     * frame.
     * @return the list of local handles as a variable length bitset.
     */
    [[nodiscard]] static EntityBitset
    getEntityBitset(ArchetypeId archetype, std::initializer_list<ComponentTypeId> changed);

    /**
     * @brief returns an array mapping from local ids inside an archetype to their respective global
     * ids.
     * @param archetype the id of the archetype to read the array from.
     * @return a reference to the array as an std::vector<EntityId> reference.
     */
    [[nodiscard]] static const std::vector<EntityId>& getGlobalEntityIds(ArchetypeId archetype);

    /**
     * @brief checks if an archetype exists.
     * @param signature the signature of the archetype as a variable length bitset.
     * @return true if the archetype was found, false otherwise.
     */
    [[nodiscard]] static bool archetypeExists(ArchetypeSignature& archetypeSignature) noexcept;

    /**
     * @brief returns the signature of an archetype.
     * @param archetype the archetype to retrieve the signature from.
     * @return the archetype's signature as a variable length bitset.
     */
    [[nodiscard]] static const ArchetypeSignature getArchetypeSignature(ArchetypeId archetype);

private:
    static std::unordered_map<ComponentTypeId, ArchetypeIdBitset> archetypeInverseIndex;
    static std::unordered_map<EntityId, ArchetypeId>              entityArchetypes;
    static std::unordered_map<std::uint64_t, Archetype>           archetypes;
    static std::unordered_map<ArchetypeSignature, std::uint64_t>  archetypeIds;
    static std::uint64_t                                          archetypeCount;
};
} // namespace Generic::Core::ECS

#endif
