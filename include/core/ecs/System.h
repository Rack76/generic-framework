#ifndef _TRIMEGO_SYSTEM_H
#define _TRIMEGO_SYSTEM_H

#include <functional>
#include <vector>
#include "EntityManager.h"

namespace Generic::Core::ECS
{
using EntityReadProc     = std::function<void(std::uint64_t entity)>;
using EntityWriteProc    = std::function<void(std::uint64_t entity)>;
using EntityProc         = std::function<void(std::uint64_t entity)>;
using OnComponentAdded   = std::function<void(void)>;
using OnComponentRemoved = std::function<void(void)>;
using Batch              = std::function<void(void)>;

class System
{
public:
    /**
     * @brief adds a read batch.
     * @param entityReadProc a procedure that processes each entity having the required component
     * types.
     * @param include the ids of the component types of the entities to process.
     * @param changed the ids of the component types of the entities that changed since beginning of
     * current frame.
     * @param exclude the ids of the component types of the entities to exclude, an entity that has
     * at least one of those types wont be processed.
     * @important !!! DO NOT USE THIS METHOD TO WRITE INTO ARCHETYPES, if you do you might have bugs
     * that are very hard to detect.
     * @note you should not store any of the component pointers inside the entity read procedure or
     * you might get dangling pointers.
     */
    void addReadBatch(EntityReadProc                         entityReadProc,
                      std::initializer_list<ComponentTypeId> include,
                      std::initializer_list<ComponentTypeId> changed = {},
                      std::initializer_list<ComponentTypeId> exclude = {});

    /**
     * @brief adds a write batch.
     * @param entityReadProc a procedure that processes each entity having the required component
     * types.
     * @param include the ids of the component types of the entities to process.
     * @param changed the ids of the component types of the entities that changed since beginning of
     * current frame.
     * @param exclude the ids of the component types of the entities to exclude, an entity that has
     * at least one of those types wont be processed.
     * @note you should not store any of the component pointers inside the entity read procedure or
     * you might get dangling pointers.
     */
    void addWriteBatch(EntityReadProc                         entityWriteProc,
                       std::initializer_list<ComponentTypeId> include,
                       std::initializer_list<ComponentTypeId> changed = {},
                       std::initializer_list<ComponentTypeId> exclude = {});

    void addOnComponentAdded(EntityProc entityProc, ComponentTypeId type);

    void addOnComponentRemoved(EntityProc entityProc, ComponentTypeId type);

    void run() noexcept;

private:
    std::vector<Batch>              batches;
    std::vector<OnComponentAdded>   onComponentAddedCallbacks;
    std::vector<OnComponentRemoved> onComponentRemovedCallbacks;
};
} // namespace Generic::Core::ECS

#endif