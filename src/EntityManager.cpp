#include "engine/managers/entityManager.hpp"

#include "engine/ecs/entity.hpp"

#include <algorithm>

// ...existing code...
namespace engine
{
    EntityManager::EntityManager() = default;

    EntityManager::~EntityManager() = default;

    ecs::Entity EntityManager::createEntity()
    {
        ecs::EntityID id;
        if (!availableIds.empty())
        {
            id = availableIds.back();
            availableIds.pop_back();
        }
        else
        {
            id = nextId++;
        }
        auto entity = std::make_unique<ecs::Entity>(id);
        entities.push_back(std::move(entity));
        signatures[id] = ecs::Signature{};
        return ecs::Entity(id);
    }

    void EntityManager::destroyEntity(ecs::Entity entity)
    {
        auto it = std::remove_if(entities.begin(), entities.end(),
                                 [entity](const std::unique_ptr<ecs::Entity>& e)
                                 { return e->id == entity.id; });
        if (it != entities.end())
        {
            entities.erase(it, entities.end());
            availableIds.push_back(entity.id);
            signatures.erase(entity.id);
        }
    }

    bool EntityManager::hasEntity(ecs::EntityID entity) const
    {
        return signatures.find(entity) != signatures.end();
    }

    const std::vector<std::unique_ptr<ecs::Entity>>& EntityManager::getEntities() const
    {
        return entities;
    }

    void EntityManager::setSignature(ecs::EntityID entity, const ecs::Signature& signature)
    {
        signatures[entity] = signature;
    }

    ecs::Signature EntityManager::getSignature(ecs::EntityID entity)
    {
        return signatures.at(entity);
    }

} // namespace engine