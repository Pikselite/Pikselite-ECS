#include <pke/ecs/managers/EntityManager.hpp>

#include <algorithm>

namespace pke
{
    namespace ecs
    {
        Entity EntityManager::createEntity()
        {
            EntityID id;

            if (!mAvailableEntities.empty())
            {
                id = mAvailableEntities.front();
                mAvailableEntities.pop_front();
            }
            else
            {
                id = mLivingEntityCount;
            }
            ++mLivingEntityCount;

            if (id < mSignaturesArr.size()) mSignaturesArr[id].reset();
            if (id < mEntities.size()) mEntities[id] = Entity(id);
            return Entity(id);
        }

        void EntityManager::destroyEntity(EntityID entity)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            if (entity < mSignaturesArr.size()) mSignaturesArr[entity].reset();

            mAvailableEntities.push_back(entity);
            if (mLivingEntityCount > 0) --mLivingEntityCount;
        }

        void EntityManager::setSignature(EntityID entity, Signature signature)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            if (entity < mSignaturesArr.size()) mSignaturesArr[entity] = signature;
        }

        Signature EntityManager::getSignature(EntityID entity) const
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            if (entity < mSignaturesArr.size()) return mSignaturesArr[entity];
            return Signature();
        }

        bool EntityManager::hasEntity(EntityID entity) const
        {
            if (entity >= MAX_ENTITIES || entity >= mSignaturesArr.size())
                return false;
            return mSignaturesArr[entity].any();
        }

        std::vector<Entity*> EntityManager::getEntities() const
        {
            std::vector<Entity*> entities;
            for (EntityID i = 0; i < MAX_ENTITIES; ++i)
            {
                if (i < mSignaturesArr.size() && mSignaturesArr[i].any())
                    entities.push_back(const_cast<Entity*>(&mEntities[i]));
            }
            return entities;
        }
    }
}
