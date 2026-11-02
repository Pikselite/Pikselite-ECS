#pragma once

#include "IComponentArray.hpp"
#include "engine/log.hpp"
#include "entity.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pke {
    namespace ecs
    {
        // Maximum number of entities that can have a specific component type (dense array capacity)
        constexpr std::size_t MAX_ENTITY_COMPONENTS = ecs::MAX_ENTITIES;

        /**
         * @brief Highest entity id the sparse lookup table will index directly.
         *
         * Entity ids come from EntityManager, which recycles freed ids, so they track the peak
         * number of live entities rather than the total ever created. Churn can still push an id
         * slightly past MAX_ENTITY_COMPONENTS (create 5000, free one, allocate 5001), hence the
         * generous headroom. The ceiling exists only so that a bogus id cannot make the lookup
         * table allocate gigabytes: at the limit it costs 4 MB per component type.
         */
        constexpr ecs::EntityID MAX_ENTITY_ID = 1u << 20;

        // Marks an entity id that owns no component in a given ComponentArray.
        constexpr std::uint32_t INVALID_COMPONENT_INDEX = 0xFFFFFFFFu;

        /**
         * @brief Dense component storage with O(1), hash-free lookup by entity id.
         *
         * Components live packed at the front of `components` (slots [0, count())), so the array
         * can be walked contiguously. Two side tables keep the mapping in both directions:
         *   - `entityToIndex` is a sparse table indexed directly by entity id;
         *   - `indexToEntity` is a dense table indexed by component slot.
         * Both are plain vectors, so a lookup is a bounds check plus one load. They used to be
         * `unordered_map`s, which made every component access hash its key.
         */
        template <typename T> class ComponentArray final : public IComponentArray
        {
        public:
            ComponentArray()
            {
                // Allocate once, construct nothing: slots are built on insert and destroyed on
                // removal. Capacity never grows past this, so the vector never reallocates and
                // references returned by getData() stay valid while other components of the same
                // type are added (a Lua create_entity() during ScriptSystem::update relies on it).
                // Untouched capacity is only reserved memory, not committed pages.
                components.reserve(MAX_ENTITY_COMPONENTS);
                indexToEntity.reserve(MAX_ENTITY_COMPONENTS);
            }
            ~ComponentArray() override = default;

            /**
             * @brief Inserts a component for a given entity ID. If the entity already has a component
             * of this type, it will be overwritten. If the maximum number of components is exceeded, an
             * exception is thrown.
             *
             * @param id
             * @param component
             */
            void insertData(ecs::EntityID id, const T& component)
            {
                const std::uint32_t existing = indexOf(id);
                if (existing != INVALID_COMPONENT_INDEX)
                {
                    components[existing] = component;
                    return;
                }

                if (size >= MAX_ENTITY_COMPONENTS)
                {
                    engine::Log::error("ComponentArray full (insert for entity " + std::to_string(id) +
                                    ")");
                    throw std::runtime_error("ComponentArray full");
                }
                if (id > MAX_ENTITY_ID)
                {
                    engine::Log::error("Entity id out of range for component storage: " +
                                    std::to_string(id));
                    throw std::runtime_error("Entity id out of range for component storage");
                }

                if (id >= entityToIndex.size())
                {
                    entityToIndex.resize(static_cast<std::size_t>(id) + 1, INVALID_COMPONENT_INDEX);
                }

                const std::uint32_t newIndex = static_cast<std::uint32_t>(size);
                components.push_back(component);
                entityToIndex[id] = newIndex;
                indexToEntity.push_back(id);
                ++size;
            }

            /**
             * @brief Removes the component associated with the given entity ID.
             * If the entity does not have a component of this type, the function does nothing.
             *
             * @param id
             */
            void removeData(ecs::EntityID id)
            {
                const std::uint32_t indexOfRemoved = indexOf(id);
                if (indexOfRemoved == INVALID_COMPONENT_INDEX)
                    return;

                const std::uint32_t indexOfLast = static_cast<std::uint32_t>(size - 1);

                if (indexOfRemoved != indexOfLast)
                {
                    components[indexOfRemoved] = std::move(components[indexOfLast]);

                    const ecs::EntityID lastEntity = indexToEntity[indexOfLast];
                    entityToIndex[lastEntity] = indexOfRemoved;
                    indexToEntity[indexOfRemoved] = lastEntity;
                }

                // Destroys the vacated slot, releasing what it owns (Sprite's std::string,
                // PhysicsBody's triangle vector).
                components.pop_back();

                entityToIndex[id] = INVALID_COMPONENT_INDEX;
                indexToEntity.pop_back();
                --size;
            }

            /**
             * @brief Get the Data object
             *
             * @param id
             * @return T&
             */
            T& getData(ecs::EntityID id)
            {
                const std::uint32_t index = indexOf(id);
                if (index == INVALID_COMPONENT_INDEX)
                {
                    engine::Log::error("Component not found for entity " + std::to_string(id));
                    throw std::out_of_range("Component not found for entity");
                }
                return components[index];
            }

            /**
             * @brief Returns a pointer to the component, or nullptr when the entity has none.
             *
             * Lets callers replace the `has(id)` + `getData(id)` pair with a single lookup.
             *
             * @param id Entity to look up.
             * @return Pointer to the component, or `nullptr`.
             */
            T* tryGetData(ecs::EntityID id)
            {
                const std::uint32_t index = indexOf(id);
                return (index == INVALID_COMPONENT_INDEX) ? nullptr : &components[index];
            }

            /**
             * @brief Checks if a component exists for the given entity ID.
             *
             * @param id
             * @return true
             * @return false
             */
            bool has(ecs::EntityID id) const
            {
                return indexOf(id) != INVALID_COMPONENT_INDEX;
            }

            /**
             * @brief Virtual implementation of hasEntityData for dynamic discovery.
             *
             * @param id
             * @return true if entity has this component
             */
            bool hasEntityData(ecs::EntityID id) const override
            {
                return has(id);
            }

            /**
             * @brief Handles the destruction of an entity by removing its associated component, if it
             * exists.
             *
             * @param id
             */
            void entityDestroyed(ecs::EntityID id) override
            {
                removeData(id);
            }

            // Dense iteration -----------------------------------------------------
            // Components occupy slots [0, count()) with no holes, so a system can walk them
            // contiguously instead of calling getComponent() once per entity. Slot order is
            // unspecified and changes on removal (swap-and-pop); do not rely on it.

            /**
             * @brief Number of components currently stored.
             * @return Count of occupied dense slots.
             */
            std::size_t count() const
            {
                return size;
            }

            /**
             * @brief Component stored at a dense slot.
             * @param denseIndex Slot in range [0, count()).
             * @return Reference to the stored component.
             */
            T& componentAt(std::size_t denseIndex)
            {
                return components[denseIndex];
            }

            /**
             * @brief Component stored at a dense slot.
             * @param denseIndex Slot in range [0, count()).
             * @return Const reference to the stored component.
             */
            const T& componentAt(std::size_t denseIndex) const
            {
                return components[denseIndex];
            }

            /**
             * @brief Entity owning the component at a dense slot.
             * @param denseIndex Slot in range [0, count()).
             * @return Owning entity id.
             */
            ecs::EntityID entityAt(std::size_t denseIndex) const
            {
                return indexToEntity[denseIndex];
            }

            /**
             * @brief Calls `fn(entityId, component&)` for every stored component.
             *
             * The callback must not add or remove components of this type: doing so reorders
             * the dense slots while they are being walked.
             *
             * @tparam Fn Callable taking `(ecs::EntityID, T&)`.
             * @param fn Callback invoked once per stored component.
             * @return void
             */
            template <typename Fn> void forEach(Fn&& fn)
            {
                for (std::size_t i = 0; i < size; ++i)
                {
                    fn(indexToEntity[i], components[i]);
                }
            }

        private:
            /**
             * @brief Resolves an entity id to its dense slot.
             * @param id Entity to resolve.
             * @return Dense slot, or INVALID_COMPONENT_INDEX when the entity has no component here.
             */
            std::uint32_t indexOf(ecs::EntityID id) const
            {
                return (id < entityToIndex.size()) ? entityToIndex[id] : INVALID_COMPONENT_INDEX;
            }

            // densely packed components, slots [0, size); capacity reserved up front (see ctor)
            std::vector<T> components;

            // sparse table: entity id -> slot in `components` (INVALID_COMPONENT_INDEX when absent)
            std::vector<std::uint32_t> entityToIndex;

            // dense table: slot in `components` -> entity id (used for swap-and-pop removal)
            std::vector<ecs::EntityID> indexToEntity;

            // number of valid components in the array
            std::size_t size = 0;
        };

    } // namespace pke { namespace ecs
}