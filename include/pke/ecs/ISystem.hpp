#pragma once

#include "entity.hpp"
#include "entitySet.hpp"



namespace pke {
    namespace pke { namespace ecs
    {
        class ComponentManager;

        /**
         * @brief The ISystem interface defines the basic structure and functionality that all systems
         * in the ECS architecture must implement.
         *
         */
        class ISystem
        {
        public:
            virtual ~ISystem() = default;

            /**
             * @brief Updates the system's state.
             *
             * @param deltaTime The time elapsed since the last update.
             * @param componentManager The component manager for accessing components.
             */
            virtual void update(double deltaTime, ComponentManager& componentManager) = 0;

            /**
             * @brief Initializes the system. This function is called once when the system is added to
             * the SystemManager.
             */
            virtual void init() = 0;

            /**
             * @brief Whether this system should be executed by SystemManager::update.
             * Rendering-only systems can override and return false.
             */
            virtual bool shouldRunInUpdate() const
            {
                return true;
            }

            /**
             * @brief Whether this system runs in SystemManager::fixedUpdate (fixed time step, in
             * lockstep with the pixel simulation) instead of SystemManager::update (once per
             * frame). Physics and movement return true so Box2D always steps by the same dt.
             */
            virtual bool runsAtFixedStep() const
            {
                return false;
            }

            /**
             * @brief Notifies the system that an entity has been destroyed.
             * Default implementation is a no-op for systems that do not own entity-local resources.
             */
            virtual void entityDestroyed(EntityID /*entity*/) {}

            /**
             * @brief Cleans up the system's state. Called when the system is being shut down or scene
             * is reset. Default implementation is a no-op.
             */
            virtual void shutdown() {}

            /**
             * @brief Entities this system is interested in, sorted by id.
             * Contiguous storage: do not add/remove entities while iterating it (see EntitySet).
             */
            EntitySet entities;
        };
    } // namespace pke::ecs
}