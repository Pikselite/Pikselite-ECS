#pragma once

#include "engine/ecs/IComponentArray.hpp"
#include "engine/ecs/componentArray.hpp"
#include "engine/ecs/componentTypeId.hpp"
#include "engine/ecs/signature.hpp"
#include "engine/log.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace pke {
    namespace engine
    {
        using ComponentType = std::uint8_t;
        // Reuse the ECS-level limit to avoid divergence with Signature bit width.
        constexpr std::size_t MAX_COMPONENT_TYPES = ecs::MAX_COMPONENT_TYPES;

        /**
         * @brief The ComponentManager class is responsible for managing the storage and access of
         * components in the ECS architecture.
         *
         * Component storage is reached through `ecs::detail::componentTypeId<T>()`, a dense index
         * assigned once per type, so resolving `T` to its array is an array index rather than a
         * hash of `typeid(T)`. Combined with the direct-indexed lookup inside ComponentArray, a
         * `getComponent<T>(entity)` call performs no hashing at all.
         */
        class ComponentManager
        {
        public:
            ComponentManager() = default;
            ~ComponentManager() = default;

            /**
             * @brief Sets the callback invoked when an entity component set changes.
             *
             * The callback is called after add/remove operations so the caller can
             * refresh signatures and system memberships.
             */
            void setEntityMutationCallback(std::function<void(ecs::EntityID)> callback)
            {
                onEntityMutated = std::move(callback);
            }

            /**
             * @brief Sets the callback invoked when a component type is removed from an entity.
             */
            void setComponentRemovalCallback(
                std::function<void(ecs::EntityID, const std::type_index&)> callback)
            {
                onComponentRemoved = std::move(callback);
            }

            /**
             * @brief Registers a component type with the manager.
             *
             * @tparam T
             */
            template <typename T> void registerComponent()
            {
                const std::size_t globalId = ecs::detail::componentTypeId<T>();

                if (globalId < slotsByGlobalId.size() && slotsByGlobalId[globalId].array != nullptr)
                {
                    engine::Log::warning(std::string("Registering component type more than once: ") +
                                        typeid(T).name());
                    return;
                }

                checkComponentCapacity();

                if (globalId >= slotsByGlobalId.size())
                {
                    slotsByGlobalId.resize(globalId + 1);
                }

                auto array = std::make_unique<ecs::ComponentArray<T>>();
                slotsByGlobalId[globalId].array = array.get();
                slotsByGlobalId[globalId].type = static_cast<ComponentType>(componentArrays.size());
                componentArrays.push_back(std::move(array));
            }

            /**
             * @brief Gets the type of a component.
             *
             * @tparam T
             * @return ComponentType
             */
            template <typename T> ComponentType getComponentType()
            {
                const std::size_t globalId = ecs::detail::componentTypeId<T>();
                if (globalId < slotsByGlobalId.size() && slotsByGlobalId[globalId].array != nullptr)
                {
                    return slotsByGlobalId[globalId].type;
                }

                engine::Log::error(std::string("Component not registered before use: ") +
                                typeid(T).name());
                throw std::runtime_error("Component not registered before use");
            }

            /**
             * @brief Adds a component to an entity.
             *
             * @tparam T
             * @param entity
             * @param component
             */
            template <typename T> void addComponent(ecs::EntityID entity, const T& component)
            {
                getComponentArray<T>()->insertData(entity, component);
                notifyEntityMutated(entity);
            }

            /**
             * @brief Removes a component from an entity.
             *
             * @tparam T
             * @param entity
             */
            template <typename T> void removeComponent(ecs::EntityID entity)
            {
                getComponentArray<T>()->removeData(entity);
                notifyComponentRemoved(entity, std::type_index(typeid(T)));
                notifyEntityMutated(entity);
            }

            /**
             * @brief Gets a reference to a component of an entity.
             *
             * @tparam T
             * @param entity
             * @return T&
             */
            template <typename T> T& getComponent(ecs::EntityID entity)
            {
                return getComponentArray<T>()->getData(entity);
            }

            /**
             * @brief Returns a pointer to a component, or nullptr when the entity has none.
             *
             * Prefer this over `hasComponent<T>(e)` followed by `getComponent<T>(e)`: it resolves
             * the storage and the entity slot once instead of twice, and it never throws — an
             * unregistered component type simply yields `nullptr`.
             *
             * @tparam T Component type.
             * @param entity Entity to look up.
             * @return Pointer to the component, or `nullptr`.
             */
            template <typename T> T* tryGetComponent(ecs::EntityID entity)
            {
                const std::size_t globalId = ecs::detail::componentTypeId<T>();
                if (globalId >= slotsByGlobalId.size())
                {
                    return nullptr;
                }

                ecs::IComponentArray* array = slotsByGlobalId[globalId].array;
                if (array == nullptr)
                {
                    return nullptr;
                }

                return static_cast<ecs::ComponentArray<T>*>(array)->tryGetData(entity);
            }

            /**
             * @brief Checks if an entity has a component of a specific type.
             *
             * @tparam T
             * @param entity
             * @return true
             * @return false
             */
            template <typename T> bool hasComponent(ecs::EntityID entity)
            {
                return getComponentArray<T>()->has(entity);
            }

            /**
             * @brief Returns the dense storage for a component type, for contiguous iteration.
             *
             * Lets a system walk every component of a type without a per-entity lookup:
             * @code
             * if (auto* velocities = componentManager.componentArray<Velocity>())
             *     velocities->forEach([](ecs::EntityID id, Velocity& v) { ... });
             * @endcode
             *
             * @tparam T Component type.
             * @return Pointer to the storage, or `nullptr` when `T` is not registered here.
             */
            template <typename T> ecs::ComponentArray<T>* componentArray()
            {
                const std::size_t globalId = ecs::detail::componentTypeId<T>();
                if (globalId >= slotsByGlobalId.size())
                {
                    return nullptr;
                }
                return static_cast<ecs::ComponentArray<T>*>(slotsByGlobalId[globalId].array);
            }

            /**
             * @brief Notifies the component manager that an entity has been destroyed, so it can remove
             * any associated components.
             *
             * @param entity
             */
            void entityDestroyed(ecs::EntityID entity)
            {
                for (auto& array : componentArrays)
                {
                    array->entityDestroyed(entity);
                }
            }

            /**
             * @brief Builds the signature for an entity by checking all registered component types.
             * This automatically discovers which components the entity has.
             *
             * @param entityId
             * @return ecs::Signature
             */
            ecs::Signature getEntitySignature(ecs::EntityID entityId) const
            {
                ecs::Signature sig{};
                // Storage index and ComponentType are the same value by construction, so the
                // slot index is the signature bit.
                for (std::size_t i = 0; i < componentArrays.size(); ++i)
                {
                    if (componentArrays[i] && componentArrays[i]->hasEntityData(entityId))
                    {
                        sig.set(i);
                    }
                }
                return sig;
            }

        private:
            // Per-manager view of one component type: where its storage lives and which
            // signature bit it owns. Indexed by ecs::detail::componentTypeId<T>().
            struct TypeSlot
            {
                // Non-owning; the unique_ptr in componentArrays owns the storage.
                ecs::IComponentArray* array = nullptr;
                ComponentType type = 0;
            };

            // Maps the process-wide component type index onto this manager's storage.
            // Sparse: a manager only fills the indices it registered itself.
            std::vector<TypeSlot> slotsByGlobalId;

            // Owns the component arrays. The position in this vector is the ComponentType
            // and therefore the signature bit index.
            std::vector<std::unique_ptr<ecs::IComponentArray>> componentArrays;

            // Optional callback used by Core to recompute entity signature on mutation.
            std::function<void(ecs::EntityID)> onEntityMutated;

            // Optional callback used by Core to react to specific component removals.
            std::function<void(ecs::EntityID, const std::type_index&)> onComponentRemoved;

            // Guard: prevent registration overflow
            void checkComponentCapacity() const
            {
                if (componentArrays.size() >= MAX_COMPONENT_TYPES)
                {
                    engine::Log::error("Maximum number of component types exceeded");
                    throw std::runtime_error("Maximum number of component types exceeded");
                }
            }

            void notifyEntityMutated(ecs::EntityID entity)
            {
                if (onEntityMutated)
                {
                    onEntityMutated(entity);
                }
            }

            void notifyComponentRemoved(ecs::EntityID entity, const std::type_index& componentType)
            {
                if (onComponentRemoved)
                {
                    onComponentRemoved(entity, componentType);
                }
            }

            /**
             * @brief Gets the component array for a specific component type.
             *
             * @tparam T
             * @return ecs::ComponentArray<T>*
             */
            template <typename T> ecs::ComponentArray<T>* getComponentArray()
            {
                const std::size_t globalId = ecs::detail::componentTypeId<T>();
                if (globalId < slotsByGlobalId.size())
                {
                    ecs::IComponentArray* array = slotsByGlobalId[globalId].array;
                    if (array != nullptr)
                    {
                        return static_cast<ecs::ComponentArray<T>*>(array);
                    }
                }

                engine::Log::error(std::string("Component not registered before use: ") +
                                typeid(T).name());
                throw std::runtime_error("Component not registered before use");
            }
        };
    } // namespace engine
}