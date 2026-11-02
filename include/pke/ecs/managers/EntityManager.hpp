#pragma once

#include "engine/ecs/entity.hpp"
#include "engine/ecs/signature.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace engine
{
    /**
     * @brief The EntityManager class is responsible for creating, destroying, and managing entities
     * in the ECS architecture. It maintains a list of active entities, a pool of available entity
     * IDs for reuse, and a mapping of entity IDs to their component signatures.
     */
    class EntityManager
    {
      public:
        EntityManager();
        ~EntityManager();

        /**
         * @brief Creates a new entity and returns its handle.
         *
         * Entity IDs are reused from the free-list when possible.
         */
        ecs::Entity createEntity();

        /**
         * @brief Destroys a given entity, making its ID available for reuse and removing its
         * signature.
         *
         * @param entity
         */
        void destroyEntity(ecs::Entity entity);

        /**
         * @brief Checks if a given entity exists.
         *
         * @param entity
         * @return true
         * @return false
         */
        bool hasEntity(ecs::EntityID entity) const;

        /**
         * @brief Gets a const reference to the list of active entities.
         *
         * @return const std::vector<std::unique_ptr<ecs::Entity>>&
         */
        const std::vector<std::unique_ptr<ecs::Entity>>& getEntities() const;

        /**
         * @brief Sets the component signature for a given entity ID.
         *
         * @param entity
         * @param signature
         */
        void setSignature(ecs::EntityID entity, const ecs::Signature& signature);

        /**
         * @brief Gets the component signature for a given entity ID.
         *
         * @param entity
         * @return ecs::Signature
         */
        ecs::Signature getSignature(ecs::EntityID entity);

      private:
        // List of active entities
        std::vector<std::unique_ptr<ecs::Entity>> entities;

        // Pool of available entity IDs for reuse
        std::vector<ecs::EntityID> availableIds;

        // Mapping of entity ID to its component signature
        std::unordered_map<ecs::EntityID, ecs::Signature> signatures;

        // Incremental entity ID generator
        ecs::EntityID nextId = 1;
    };
} // namespace engine