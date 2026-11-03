#pragma once

namespace pke { namespace ecs::components
{
    /**
     * @brief The Velocity component represents the speed and direction of an entity's movement in
     * 2D space. It contains properties for the x and y velocity components.
     */
    struct Velocity
    {
        bool enabled = true;

        float vx, vy;
    };
} // namespace pke::ecs::components