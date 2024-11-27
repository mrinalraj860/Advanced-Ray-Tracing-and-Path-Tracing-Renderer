#pragma once
#include "Ray.hpp"

// Define a simple point light class
class Light
{
public:
    Vec3 position;  // Position of the light in the scene
    Vec3 intensity; // Intensity of the light

    // Constructor initializing the position and intensity of the light
    Light(const Vec3 &pos, const Vec3 &intensity)
        : position(pos), intensity(intensity) {}
};