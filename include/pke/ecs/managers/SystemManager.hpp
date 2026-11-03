#pragma once

#include "engine/ecs/ISystem.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"
#include "engine/ecs/signature.hpp"
#include "engine/ecs/systems/movementSystem.hpp"
#include "engine/managers/componentManager.hpp"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace engine
{
    /**
     * @brief The SystemManager class is responsible for managing all systems in the ECS
     * architecture. It allows adding systems, updating them each frame, and notifying them of
     * changes to entity signatures or when entities are destroyed.
     */
    class SystemManager
    {
      public:
        /**
         * @brief Updates all systems by calling their update function with the given delta time and
         * component manager.
         *
         * @param deltaTime
         * @param componentManager
         */
        void update(double deltaTime, ComponentManager& componentManager);

        /**
         * @brief Updates the systems that run at a fixed time step (runsAtFixedStep()).
         * Call it once per fixed tick, with the fixed tick duration.
         *
         * @param fixedDeltaTime
         * @param componentManager
         */
        void fixedUpdate(double fixedDeltaTime, ComponentManager& componentManager);

        /**
         * @brief Notifies systems that an entity's signature has changed, so they can update their
         * list of relevant entities accordingly.
         *
         * @param entity
         * @param entitySignature
         */
        void entitySignatureChanged(ecs::EntityID entity, const ecs::Signature& entitySignature);

        /**
         * @brief Notifies systems that an entity has been destroyed, so they can remove it from
         * their lists if necessary.
         *
         * @param entity
         */
        void entityDestroyed(ecs::EntityID entity);

        /**
         * @brief Shuts down all systems, clearing their state.
         */
        void shutdownAll()
        {
            for (auto& system : systems)
            {
                system->shutdown();
                // Callers destroy every entity right after this (scene reload). Emptying the
                // membership lists first turns those per-entity erases into no-ops instead of
                // shifting the whole sorted vector each time (O(n^2) for a full unload).
                // entityDestroyed() still reaches each system, so per-entity cleanup
                // (e.g. PhysicsSystem freeing Box2D bodies) is unchanged.
                system->entities.clear();
            }
        }

        /**
         * @brief Adds a new system of type T to the manager and returns a reference to it.
         *
         * @tparam T
         * @tparam Args
         * @param args
         * @return T&
         */
        template <typename T, typename... Args> T& addSystem(Args&&... args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *system;

            systemsMap[typeid(T)] = system.get();
            systems.push_back(std::move(system));
            return ref;
        }

        /**
         * @brief Sets the signature for a system of type T, which defines which components an
         * entity must have to be processed by that system.
         *
         * @tparam T
         * @param signature
         */
        template <typename T> void setSignature(const ecs::Signature& signature)
        {
            systemSignatures[typeid(T)] = signature;
        }

        /**
         * @brief Gets a pointer to a system of type T. Returns nullptr if the system does not
         * exist.
         *
         * @tparam T
         * @return T*
         */
        template <typename T> T* getSystem()
        {
            for (auto& system : systems)
            {
                if (auto ptr = dynamic_cast<T*>(system.get()))
                    return ptr;
            }
            return nullptr;
        }

      private:
        // List of all systems managed by the SystemManager
        std::vector<std::unique_ptr<ecs::ISystem>> systems;

        // Mapping from system type to system instance for quick access
        std::unordered_map<std::type_index, ecs::ISystem*> systemsMap;

        // Mapping from system type to its required component signature
        std::unordered_map<std::type_index, ecs::Signature> systemSignatures;
    };
} // namespace engine