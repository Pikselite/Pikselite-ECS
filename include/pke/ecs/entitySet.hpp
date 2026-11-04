#pragma once

#include "entity.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace pke
{
    namespace ecs
    {
        /**
         * @brief Sorted, duplicate-free set of entity ids stored in one contiguous vector.
         *
         * Drop-in replacement for `std::set<EntityID>` in systems: same operations, same
         * ascending iteration order, but no per-entity node allocation and contiguous iteration.
         * Insert and erase are a binary search plus a shift; ids are mostly created in increasing
         * order, so inserts usually land at the end.
         *
         * Unlike std::set, inserting or erasing invalidates every iterator. Do not modify the set
         * while looping over it; loop over a copy when the loop body can add or remove entities
         * (see ScriptSystem::update).
         */
        class EntitySet
        {
        public:
            // Plain pointers rather than vector iterators: MSVC Debug builds check every vector
            // iterator increment/dereference, which made system loops slower than std::set there.
            using const_iterator = const EntityID*;

            /**
             * @brief Adds an id if it is not already present.
             * @return true if the id was inserted, false if it was already there.
             */
            bool insert(EntityID id)
            {
                const auto it = std::lower_bound(_ids.begin(), _ids.end(), id);
                if (it != _ids.end() && *it == id)
                    return false;
                _ids.insert(it, id);
                return true;
            }

            /**
             * @brief Removes an id.
             * @return Number of ids removed (0 or 1), as std::set::erase.
             */
            std::size_t erase(EntityID id)
            {
                const auto it = std::lower_bound(_ids.begin(), _ids.end(), id);
                if (it == _ids.end() || *it != id)
                    return 0;
                _ids.erase(it);
                return 1;
            }

            bool contains(EntityID id) const
            {
                return std::binary_search(_ids.begin(), _ids.end(), id);
            }

            /**
             * @brief Number of occurrences of an id (0 or 1), as std::set::count.
             */
            std::size_t count(EntityID id) const
            {
                return contains(id) ? 1 : 0;
            }

            std::size_t size() const
            {
                return _ids.size();
            }

            bool empty() const
            {
                return _ids.empty();
            }

            void clear()
            {
                _ids.clear();
            }

            const_iterator begin() const
            {
                return _ids.data();
            }

            const_iterator end() const
            {
                return _ids.data() + _ids.size();
            }

            /**
             * @brief Underlying sorted ids, e.g. to take a snapshot before a mutating loop.
             */
            const std::vector<EntityID>& ids() const
            {
                return _ids;
            }

        private:
            std::vector<EntityID> _ids;
        };
    } // namespace pke::ecs
}