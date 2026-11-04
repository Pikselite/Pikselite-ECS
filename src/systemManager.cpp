#include <pke/ecs/managers/SystemManager.hpp>
#include <pke/ecs/ISystem.hpp>

namespace pke
{
    namespace ecs
    {
        void SystemManager::entityDestroyed(EntityID entity)
        {
            for (auto const& pair : systemsMap)
            {
                auto const& system = pair.second;

                system->entities.erase(entity);
            }
        }

        void SystemManager::entitySignatureChanged(EntityID entity, const Signature& entitySignature)
        {
            for (auto const& pair : systemsMap)
            {
                auto const& type = pair.first;
                auto const& system = pair.second;
                auto const& systemSignature = systemSignatures[type];

                if ((entitySignature & systemSignature) == systemSignature)
                {
                    system->entities.insert(entity);
                }
                else
                {
                    system->entities.erase(entity);
                }
            }
        }

        void SystemManager::update(double deltaTime, ComponentManager& componentManager)
        {
            for (auto& system : systems)
            {
                if (!system->shouldRunInUpdate())
                    continue;
                if (system->runsAtFixedStep())
                    continue;
                system->update(deltaTime, componentManager);
            }
        }

        void SystemManager::fixedUpdate(double fixedDeltaTime, ComponentManager& componentManager)
        {
            for (auto& system : systems)
            {
                if (!system->shouldRunInUpdate())
                    continue;
                if (!system->runsAtFixedStep())
                    continue;
                system->update(fixedDeltaTime, componentManager);
            }
        }
    } // namespace ecs
} // namespace pke
