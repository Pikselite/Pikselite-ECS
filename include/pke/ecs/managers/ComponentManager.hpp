#pragma once

#include <pke/ecs/IComponentArray.hpp>
#include <pke/ecs/componentArray.hpp>
#include <pke/ecs/componentTypeId.hpp>
#include <pke/ecs/signature.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include <iostream>
#include <cassert>

namespace pke {
    namespace ecs
    {
        using ComponentType = std::uint8_t;

        class ComponentManager
        {
        public:
            ComponentManager() = default;
            ~ComponentManager() = default;

            template<typename T>
            void registerComponent()
            {
                const char* typeName = typeid(T).name();

                assert(mComponentTypes.find(typeName) == mComponentTypes.end() && "Registering component type more than once.");

                ComponentType id = mNextComponentType++;
                mComponentTypes.insert({typeName, id});
                mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

                if (mNextComponentType > MAX_COMPONENT_TYPES)
                {
                    std::cerr << "MAX_COMPONENT_TYPES exceeded\n";
                }
            }

            template<typename T>
            ComponentType getComponentType()
            {
                const char* typeName = typeid(T).name();

                assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

                return mComponentTypes[typeName];
            }

            template<typename T>
            void addComponent(EntityID entity, T component)
            {
                getComponentArray<T>()->insertData(entity, component);
                if (mEntityMutationCallback)
                    mEntityMutationCallback(entity);
            }

            template<typename T>
            void removeComponent(EntityID entity)
            {
                getComponentArray<T>()->removeData(entity);
                if (mComponentRemovalCallback)
                    mComponentRemovalCallback(entity, std::type_index(typeid(T)));
                if (mEntityMutationCallback)
                    mEntityMutationCallback(entity);
            }

            template<typename T>
            T& getComponent(EntityID entity)
            {
                return getComponentArray<T>()->getData(entity);
            }

            template<typename T>
            const T& getComponent(EntityID entity) const
            {
                return getComponentArray<T>()->getData(entity);
            }

            template<typename T>
            T* tryGetComponent(EntityID entity)
            {
                auto array = getComponentArray<T>();
                if (!array) return nullptr;
                return array->tryGetData(entity);
            }

            template<typename T>
            const T* tryGetComponent(EntityID entity) const
            {
                auto array = getComponentArray<T>();
                if (!array) return nullptr;
                return array->tryGetData(entity);
            }

            void entityDestroyed(EntityID entity)
            {
                for (auto const& pair : mComponentArrays)
                {
                    auto const& component = pair.second;
                    component->entityDestroyed(entity);
                }
            }

            Signature getEntitySignature(EntityID entity) const
            {
                Signature sig = {};
                for (auto const& pair : mComponentTypes)
                {
                    const char* typeName = pair.first;
                    ComponentType typeId = pair.second;
                    auto arrIt = mComponentArrays.find(typeName);
                    if (arrIt == mComponentArrays.end())
                        continue;
                    if (arrIt->second && arrIt->second->hasEntityData(entity))
                    {
                        sig.set(typeId);
                    }
                }
                return sig;
            }

            void setEntityMutationCallback(std::function<void(EntityID)> cb)
            {
                mEntityMutationCallback = std::move(cb);
            }

            void setComponentRemovalCallback(std::function<void(EntityID, const std::type_index&)> cb)
            {
                mComponentRemovalCallback = std::move(cb);
            }

        private:
            template<typename T>
            std::shared_ptr<ComponentArray<T>> getComponentArray()
            {
                const char* typeName = typeid(T).name();

                assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

                return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
            }

            template<typename T>
            std::shared_ptr<ComponentArray<T>> getComponentArray() const
            {
                const char* typeName = typeid(T).name();

                auto itType = mComponentTypes.find(typeName);
                if (itType == mComponentTypes.end()) return nullptr;
                auto itArr = mComponentArrays.find(typeName);
                if (itArr == mComponentArrays.end()) return nullptr;
                return std::static_pointer_cast<ComponentArray<T>>(itArr->second);
            }

            std::unordered_map<const char*, ComponentType> mComponentTypes{};
            std::unordered_map<const char*, std::shared_ptr<IComponentArray>> mComponentArrays{};
            ComponentType mNextComponentType{};
            std::function<void(EntityID)> mEntityMutationCallback;
            std::function<void(EntityID, const std::type_index&)> mComponentRemovalCallback;
        };
    } // namespace ecs
} // namespace pke
