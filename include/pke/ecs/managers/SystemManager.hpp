#pragma once

#include <pke/ecs/ISystem.hpp>
#include <pke/ecs/signature.hpp>
#include <pke/ecs/managers/ComponentManager.hpp>

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace pke
{
    namespace ecs
    {
        class SystemManager
        {
        public:
            void update(double deltaTime, ComponentManager& componentManager);

            void fixedUpdate(double fixedDeltaTime, ComponentManager& componentManager);

            void entitySignatureChanged(EntityID entity, const Signature& entitySignature);

            void entityDestroyed(EntityID entity);

            void shutdownAll()
            {
                for (auto& system : systems)
                {
                    system->shutdown();
                    system->entities.clear();
                }
            }

            template <typename T, typename... Args> T& addSystem(Args&&... args)
            {
                auto system = std::make_unique<T>(std::forward<Args>(args)...);
                T& ref = *system;
                systemsMap[typeid(T)] = system.get();
                systems.push_back(std::move(system));
                return ref;
            }

            template <typename T> void setSignature(const Signature& signature)
            {
                systemSignatures[typeid(T)] = signature;
            }

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
            std::vector<std::unique_ptr<ISystem>> systems;
            std::unordered_map<std::type_index, ISystem*> systemsMap;
            std::unordered_map<std::type_index, Signature> systemSignatures;
        };
    } // namespace ecs
} // namespace pke
