#ifndef _GENERIC_COMPONENT_MANAGER_H_
#define _GENERIC_COMPONENT_MANAGER_H_

#include <memory>
#include <unordered_map>
#include "../ObjectPool.h"
#include "util/CTTI.h"

using BlockHandle     = std::uint64_t;
using ArchetypeId     = std::uint64_t;
using ComponentTypeId = std::uint64_t;

namespace Generic::Core::ECS
{
class ComponentManager
{
public:
    /**
     * @brief Registers the type `T` as a component in the ECS.
     * @note Must be called during initialization, before `T` is used in the other ECS functions.
     * @details please do not derive T from another class, as this is useless and makes an instance
     * of T occupy more memory which is not cache friendly. keep your component types small and
     * decoupled.
     */
    template <typename T>
    static void registerComponentType()
    {
        pools[Util::typeId<T>()]     = std::make_shared<ObjectPoolTemplate<T>>();
        typeNames[Util::typeId<T>()] = Util::typeName<T>();
    }

    /**
     * @brief returns a handle to a block inside an object pool.
     * @param componentTypeId the component type id of the object pool's type.
     * @return a handle to block which contains instances of the requested pool type.
     */
    [[nodiscard]] static BlockHandle getBlockHandle(ComponentTypeId componentTypeId);

    /**
     * @brief reserves a component, initializes it, and returns a pointer to it.
     * @param block a handle to a block inside an object pool.
     * @param componentTypeId the component type id of the reserved component.
     * @param value a pointer to the value to initialize the component with.
     * @return a pointer to the reserved component.
     */
    [[nodiscard]] static void*
    getComponent(BlockHandle block, ComponentTypeId componentTypeId, void* value);

    /**
     * @brief reserves a component, and returns a pointer to it.
     * @param block a handle to a block inside an object pool.
     * @param componentTypeId the component type id of the reserved component.
     * @return a pointer to the reserved component.
     */
    [[nodiscard]] static void* getComponent(BlockHandle block, ComponentTypeId componentTypeId);
    /**
     * @brief copy an component to its destination in memory.
     * @param block a handle to a block inside an object pool.
     * @param dest a pointer to the destination to copy the component to.
     * @param source a pointer to the source component.
     */
    static void copyComponent(BlockHandle     block,
                              ComponentTypeId componentTypeId,
                              void*           dest,
                              void*           source) noexcept;
    /**
     * @brief frees an reserved component.
     * @param blockHandle a handle to a block inside an object pool.
     * @param componentTypeId the component type id of the reserved component.
     * @param c a pointer to the object to free.
     */
    static void
    returnComponent(ComponentTypeId componentTypeId, BlockHandle blockHandle, void* c) noexcept;

private:
    static std::unordered_map<ComponentTypeId, std::shared_ptr<ObjectPool>> pools;
    static std::unordered_map<ComponentTypeId, BlockHandle>                 blockHandleCounters;
    static std::unordered_map<ComponentTypeId, std::string>                 typeNames;
};
} // namespace Generic::Core::ECS

#endif