#pragma once

#include <cstddef>

namespace pke
{
    namespace ecs
    {
        namespace detail
        {
            /**
             * @brief Process-wide counter handing out one dense index per component type.
             * @return Reference to the shared counter.
             */
            inline std::size_t& componentTypeIdCounter()
            {
                static std::size_t counter = 0;
                return counter;
            }

            /**
             * @brief Returns a dense index unique to `T`, assigned on first use.
             *
             * This replaces `std::type_index(typeid(T))` as the key used to reach a
             * component's storage: a ComponentManager can index an array with it
             * instead of hashing a type_index, which is what made every
             * `getComponent<T>()` pay for a hash lookup.
             *
             * The index is global to the process, not to a manager. Each
             * ComponentManager maps it onto its own local slot, so two managers may
             * register different sets of component types without interfering: a
             * manager only ever looks at the indices it registered itself.
             *
             * @tparam T Component type.
             * @return Stable index for `T`, identical for every call in this process.
             */
            template <typename T> inline std::size_t componentTypeId()
            {
                static const std::size_t id = componentTypeIdCounter()++;
                return id;
            }
        } // namespace detail
    } // namespace ecs
}