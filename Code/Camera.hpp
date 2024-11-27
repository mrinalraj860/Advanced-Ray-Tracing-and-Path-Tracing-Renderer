#pragma once
#include <cmath> // For tan and other mathematical functions
#include "utility.hpp"

class Camera
{
public:
    Vec3 origin;
    Vec3 lowerLeftCorner;
    Vec3 horizontal;
    Vec3 vertical;
    float exposure;
    Vec3 u, v, w;
    int width;
    int height;
    float camera_radius;
    Camera()
    {
    }

    Camera(const Vec3 &lookFrom, const Vec3 &lookAt, const Vec3 &up, float fov, float aspect, float aperture = 0.1, int wi = 1500, int he = 800)
        : origin(lookFrom), width(wi), height(he)
    {
        float theta = degrees_to_radians(fov);
        float halfHeight = tan(theta / 2);
        float viewport_height = 2.0 * halfHeight;
        float viewport_width = aspect * viewport_height;

        w = unit(lookFrom - lookAt);
        u = unit(w.cross(up));
        v = w.cross(u);

        horizontal = viewport_width * u;
        vertical = viewport_height * v;
        lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - w;

        camera_radius = aperture / 200; // Aperture directly affects camera radius, not exposure
    }

    Ray get_ray(double s, double t) const
    {
        Vec3 rd = camera_radius * Camera_Sampling();
        Vec3 offset = u * rd.x + v * rd.y;
        return Ray(origin + offset, lowerLeftCorner + s * horizontal + t * vertical - origin - offset);
    }
    float getCameraRadius() const
    {
        return camera_radius;
    }
};