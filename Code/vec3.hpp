#pragma once
#include <cmath>
#include <iostream>
#include "json/include/nlohmann/json.hpp"
#include "utility.hpp"

class Vec3
{
public:
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3(const nlohmann::json &json_array)
        : x(json_array.at(0).get<float>()),
          y(json_array.at(1).get<float>()),
          z(json_array.at(2).get<float>()) {}
    inline float getX() const { return x; }
    inline float getY() const { return y; }
    inline float getZ() const { return z; }
    Vec3 operator+(const Vec3 &v) const
    {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    Vec3 operator-() const
    {
        return Vec3(-x, -y, -z);
    }

    Vec3 &operator+=(const Vec3 &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 operator-(const Vec3 &v) const
    {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    Vec3 operator*(float scalar) const
    {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    Vec3 operator*(const Vec3 &other) const
    {
        return Vec3(x * other.x, y * other.y, z * other.z);
    }

    Vec3 &operator*=(const Vec3 &other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    Vec3 operator/(float scalar) const
    {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    Vec3 &operator/=(const Vec3 &other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    Vec3 &operator/=(const double t)
    {
        return *this *= 1 / t;
    }

    double length_squared() const
    {
        return x * x + y * y + z * z;
    }

    double length() const
    {
        return sqrt(length_squared());
    }

    Vec3 cross(const Vec3 &v) const
    {
        return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    float dot(const Vec3 &v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec3 normalized() const
    {
        float len = sqrt(x * x + y * y + z * z);
        return *this / len;
    }
    inline static Vec3 random()
    {
        return Vec3(random_double(), random_double(), random_double());
    }

    inline static Vec3 random(double min, double max)
    {
        return Vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }
};
inline std::ostream &operator<<(std::ostream &out, const Vec3 &v)
{
    return out << v.x << ' ' << v.y << ' ' << v.z << std::endl;
}

inline Vec3 unit(const Vec3 &v)
{
    return v / v.length();
}

Vec3 random_in_unit_sphere()
{
    while (true)
    {
        auto p = Vec3::random(-1, 1);
        if (p.length_squared() >= 1)
            continue;
        return p;
    }
}

Vec3 Camera_Sampling()
{
    while (true)
    {
        auto p = Vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() >= 1)
            continue;
        return p;
    }
}

Vec3 random_unit_vector()
{
    auto a = random_double(0, 2 * pi);
    auto z = random_double(-1, 1);
    auto r = sqrt(1 - z * z);
    return Vec3(r * cos(a), r * sin(a), z);
}

Vec3 random_in_hemisphere(const Vec3 &normal)
{
    Vec3 in_unit_sphere = random_in_unit_sphere();
    if (in_unit_sphere.dot(normal) > 0.0)
    {
        // In the same hemisphere as the normal.
        return in_unit_sphere;
    }
    else
    {
        return -in_unit_sphere;
    }
}

Vec3 reflect(const Vec3 &v, const Vec3 &n)
{
    return v - n * 2 * v.dot(n);
}

inline Vec3 operator*(double t, const Vec3 &v)
{
    return Vec3(t * v.x, t * v.y, t * v.z);
}

Vec3 refract(const Vec3 &uv, const Vec3 &n, double etai_over_etat)
{
    auto cos_theta = (-uv).dot(n);
    Vec3 r_out_paralell = etai_over_etat * (uv + cos_theta * n);
    Vec3 r_out_perp = -sqrt(1.0 - r_out_paralell.length_squared()) * n;
    return r_out_paralell + r_out_perp;
}
