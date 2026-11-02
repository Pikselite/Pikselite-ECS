#pragma once
#include "engine/ecs/componentArray.hpp"

#include <bitset>

// A signature is a bitset where each bit represents whether an entity has a specific component type
// or not. Using MAX_COMPONENT_TYPES (32) instead of MAX_ENTITIES allows for efficient bitset
// operations.
namespace pke {
    namespace ecs
    {
        // Single source of truth for the number of distinct component types.
        constexpr std::size_t MAX_COMPONENT_TYPES = 32;

        // We use this instead of MAX_ENTITIES (5000) to keep signatures small and efficient (4 bytes vs
        // 625 bytes)
        using Signature = std::bitset<MAX_COMPONENT_TYPES>;
    } // namespace pke { namespace ecs
}