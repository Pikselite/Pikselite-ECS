#include <pke/ecs/managers/EntityManager.hpp>

#include <algorithm>

namespace pke
{
    namespace ecs
    {
        EntityID EntityManager::createEntity()
        {
            EntityID id;

            if (mLivingEntityCount < MAX_ENTITIES)
            {
                id = mAvailableEntities.front();
                mAvailableEntities.pop_front();
                ++mLivingEntityCount;
            }
            else
            {
                id = mLivingEntityCount;
                ++mLivingEntityCount;
            }

            if (id < mSignaturesArr.size()) mSignaturesArr[id].reset();
            return id;
        }

        void EntityManager::destroyEntity(EntityID entity)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            if (entity < mSignaturesArr.size()) mSignaturesArr[entity].reset();

            mAvailableEntities.push_back(entity);
            --mLivingEntityCount;
        }

        void EntityManager::setSignature(EntityID entity, Signature signature)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            if (entity < mSignaturesArr.size()) mSignaturesArr[entity] = signature;
        }

        Signature EntityManager::getSignature(EntityID entity)
        {
            assert(entity < MAX_ENTITIES && "Entity out of range.");

            if (entity < mSignaturesArr.size()) return mSignaturesArr[entity];
            return Signature();
        }

        bool EntityManager::hasEntity(EntityID entity)
        {
            if (entity >= MAX_ENTITIES || entity >= mSignaturesArr.size())
                return false;
            return mSignaturesArr[entity].any();
        }

        std::vector<Entity*> EntityManager::getEntities()
        {
            std::vector<Entity*> entities;
            for (EntityID i = 0; i < MAX_ENTITIES; ++i)
            {
                if (i < mSignaturesArr.size() && mSignaturesArr[i].any())
                    entities.push_back(&mEntities[i]);
            }
            return entities;
        }
    }
}
