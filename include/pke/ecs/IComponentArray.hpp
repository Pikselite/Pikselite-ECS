#pragma once
#include "entity.hpp"

namespace pke {
    namespace pke { namespace ecs
    {
        /**
         * @brief The IComponentArray interface defines the basic operations that a component array must
         * implement. This includes handling the destruction of entities and providing a virtual
         * destructor for proper cleanup.
         */
        struct IComponentArray
        {
            virtual ~IComponentArray() = default;
            virtual void entityDestroyed(ecs::EntityID id) = 0;

            /**
             * @brief Checks if an entity has a component in this array.
             * Used for dynamic signature discovery.
             *
             * @param id Entity ID to check
             * @return true if entity has this component type
             */
            virtual bool hasEntityData(ecs::EntityID id) const = 0;
        };
    } // namespace pke::ecs
}