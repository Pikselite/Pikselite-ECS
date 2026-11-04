#pragma once
#include <cstdint>

namespace pke
{
    namespace ecs
    {
        // An Entity is simply an ID. It can have multiple components associated with it, but it has no
        // data itself.
        using EntityID = std::uint32_t;

        // Maximum number of entities in the system. This is used to size arrays and can be adjusted as
        // needed.
        constexpr EntityID MAX_ENTITIES = 5000;

        /**
         * @brief The Entity struct represents a game object in the ECS architecture.
         * The Entity struct provides basic comparison operators to allow for easy comparison of
         * entities.
         */
        struct Entity
        {
            EntityID id;
            explicit Entity(EntityID id = 0) : id(id) {}
            bool operator==(const Entity& other) const
            {
                return id == other.id;
            }
            bool operator!=(const Entity& other) const
            {
                return id != other.id;
            }
        };
    } // namespace pke::ecs
}
