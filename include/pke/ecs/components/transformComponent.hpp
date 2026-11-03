#pragma once

namespace pke { namespace ecs::components
{
    /**
     * @brief The Transform component represents the position, rotation, and scale of an entity in
     * 2D space. It contains properties for the x and y coordinates, rotation angle, and scale
     * factors for both axes.
     */
    struct Transform
    {
        bool enabled = true;

        float x, y;
        float rotation;
        float scaleX, scaleY;

        // Previous frame position, used to compute movement delta
        float prevX = 0.0f, prevY = 0.0f;
    };
} // namespace pke::ecs::components
