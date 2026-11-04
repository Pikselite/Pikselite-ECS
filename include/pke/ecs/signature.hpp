#pragma once

#include <bitset>
#include <cstdint>

namespace pke
{
    namespace ecs
    {
        constexpr std::uint32_t MAX_COMPONENT_TYPES = 32;
        using Signature = std::bitset<MAX_COMPONENT_TYPES>;
    } // namespace ecs
} // namespace pke
