#pragma once
#include "Hitable.hpp"
#include "utility.hpp"
#include <cmath>
#include <memory>

class Triangle : public Hittable
{
public:
    Vec3 v1, v2, v3; // Vertices of the triangle
    std::shared_ptr<Material> mat_ptr;

    Triangle() {}

    Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, std::shared_ptr<Material> m)
        : v1(p1), v2(p2), v3(p3), mat_ptr(m) {}

    bool hit(const Ray &r, double t_min, double t_max, Hit_record &rec) const override
    {
        // Implements the Möller-Trumbore intersection algorithm
        const double EPSILON = 1e-6;
        Vec3 edge1 = v2 - v1;
        Vec3 edge2 = v3 - v1;
        Vec3 h = r.direction.cross(edge2);
        double a = edge1.dot(h);

        // If a is close to 0, the ray is parallel to the triangle.
        if (a > -EPSILON && a < EPSILON)
            return false;

        double f = 1.0 / a;
        Vec3 s = r.origin - v1;
        double u = f * s.dot(h);

        // Check if intersection lies outside the triangle
        if (u < 0.0 || u > 1.0)
            return false;

        Vec3 q = s.cross(edge1);
        double v = f * r.direction.dot(q);

        // Check if intersection lies outside the triangle
        if (v < 0.0 || u + v > 1.0)
            return false;

        // Calculate t to find where the intersection point is on the ray
        double t = f * edge2.dot(q);

        if (t < t_min || t > t_max)
            return false;

        // Update the hit record with intersection details
        rec.t = t;
        rec.p = r.at(t);
        Vec3 outward_normal = edge1.cross(edge2).normalized();
        rec.set_face_normal(r, outward_normal);
        rec.mat_ptr = mat_ptr;

        return true;
    }
    bool bounding_box(double t0, double t1, aabb &output_box) const override;
};

bool Triangle::bounding_box(double t0, double t1, aabb &output_box) const
{
    // Compute the center and radius for the bounding sphere
    Vec3 center = (v1 + v2 + v3) / 3;
    double radius = std::max({(center - v1).length(), (center - v2).length(), (center - v3).length()});

    // Create an AABB (Axis-Aligned Bounding Box) around the triangle
    output_box = aabb(center - Vec3(radius, radius, radius), center + Vec3(radius, radius, radius));
    return true;
}
