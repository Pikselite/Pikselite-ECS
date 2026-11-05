#pragma once

#include <pke/ecs/entity.hpp>
#include <pke/ecs/signature.hpp>
#include <array>
#include <cassert>
#include <deque>
#include <vector>

namespace pke
{
    namespace ecs
    {
        class EntityManager
        {
        public:
            EntityManager()
            {
                for (EntityID entity = 0; entity < MAX_ENTITIES; ++entity)
                {
                    mEntities[entity] = Entity(entity);
                    mAvailableEntities.push_back(entity);
                }
                mSignaturesArr.assign(MAX_ENTITIES, Signature());
            }

            Entity createEntity();
            void destroyEntity(EntityID entity);
            void destroyEntity(const Entity& entity) { destroyEntity(entity.id); }
            void setSignature(EntityID entity, Signature signature);
            Signature getSignature(EntityID entity) const;
            bool hasEntity(EntityID entity) const;
            bool hasEntity(const Entity& entity) const { return hasEntity(entity.id); }
            std::vector<Entity*> getEntities() const;

        private:
            std::deque<EntityID> mAvailableEntities;
            std::array<Entity, MAX_ENTITIES> mEntities;
            std::vector<Signature> mSignaturesArr;
            uint32_t mLivingEntityCount = 0;
        };
    }
}
