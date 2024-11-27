#pragma once
#include "Hitable.hpp"
#include "utility.hpp"
#include <cmath>
#include <memory>

class Cylinder : public Hittable
{
public:
    Vec3 center; // Center point of the cylinder (midpoint of the axis)
    Vec3 axis;   // Normalized axis of the cylinder (from base to top)
    double radius;
    double height;
    std::shared_ptr<Material> mat_ptr;

    Cylinder() {}

    Cylinder(const Vec3 &c, const Vec3 &a, double r, double h, std::shared_ptr<Material> m)
        : center(c), axis(a.normalized()), radius(r), height(h), mat_ptr(m) {}

    bool hit(const Ray &r, double t_min, double t_max, Hit_record &rec) const override
    {
        // Define the top and bottom centers based on the full height along the axis
        Vec3 base_center = center - height * axis;
        Vec3 top_center = center + height * axis;

        // Intersection with curved surface of the cylinder
        Vec3 oc = r.origin - base_center;
        Vec3 w = axis * axis.dot(r.direction); // Parallel component along axis
        Vec3 d = r.direction - w;              // Perpendicular component to axis
        Vec3 v = oc - axis * axis.dot(oc);     // Projected origin onto the plane

        double a = d.dot(d);
        double half_b = v.dot(d);
        double c = v.dot(v) - radius * radius;
        double discriminant = half_b * half_b - a * c;

        if (discriminant > 0)
        {
            double sqrt_d = std::sqrt(discriminant);
            double t = (-half_b - sqrt_d) / a;

            if (t < t_min || t > t_max)
            {
                t = (-half_b + sqrt_d) / a;
                if (t < t_min || t > t_max)
                    return false;
            }

            Vec3 hit_point = r.at(t);
            double projection = (hit_point - base_center).dot(axis);

            if (projection >= 0 && projection <= 2 * height)
            {
                rec.t = t;
                rec.p = hit_point;
                rec.normal = ((hit_point - base_center) - axis * projection).normalized();
                rec.mat_ptr = mat_ptr;
                return true;
            }
        }

        // Check intersection with the bottom cap
        if (intersect_caps(r, t_min, t_max, rec, base_center, false))
            return true;

        // Check intersection with the top cap
        if (intersect_caps(r, t_min, t_max, rec, top_center, true))
            return true;

        return false;
    }

    virtual bool bounding_box(double t0, double t1, aabb &output_box) const override
    {
        Vec3 base_center = center - height * axis;
        Vec3 top_center = center + height * axis;

        // Calculate min and max bounds by expanding along cylinder's radius
        Vec3 min_bound(
            std::fmin(base_center.x - radius, top_center.x - radius),
            std::fmin(base_center.y - radius, top_center.y - radius),
            std::fmin(base_center.z - radius, top_center.z - radius));
        Vec3 max_bound(
            std::fmax(base_center.x + radius, top_center.x + radius),
            std::fmax(base_center.y + radius, top_center.y + radius),
            std::fmax(base_center.z + radius, top_center.z + radius));

        output_box = aabb(min_bound, max_bound);
        return true;
    }

    bool intersect_caps(const Ray &r, double t_min, double t_max, Hit_record &rec, const Vec3 &cap_center, bool is_top) const
    {
        // Project ray direction onto the cylinder's axis to find intersection with cap plane
        double denom = r.direction.dot(axis);
        if (std::fabs(denom) > 1e-6)
        { // Avoid division by zero, ray parallel to cap plane
            double t = (cap_center - r.origin).dot(axis) / denom;
            if (t < t_min || t > t_max)
                return false;

            Vec3 point = r.at(t);
            double dist_from_center = (point - cap_center).length();

            if (dist_from_center <= radius)
            {
                rec.t = t;
                rec.p = point;
                rec.normal = is_top ? axis : -axis; // Normal points outwards from the cap
                rec.mat_ptr = mat_ptr;
                return true;
            }
        }
        return false;
    }
};
// class Cylinder : public Hittable
// {
// public:
//     Vec3 center; // Center of the cylinder's base
//     Vec3 axis;   // Normalized axis of the cylinder (from base to top)
//     double radius;
//     double height;
//     std::shared_ptr<Material> mat_ptr;

//     Cylinder() {}

//     Cylinder(const Vec3 &c, const Vec3 &a, double r, double h, std::shared_ptr<Material> m)
//         : center(c), axis(a.normalized()), radius(r), height(h), mat_ptr(m) {}

//     bool hit(const Ray &r, double t_min, double t_max, Hit_record &rec) const override
//     {
//         // Intersection with curved surface of the cylinder
//         Vec3 oc = r.origin - center;
//         Vec3 w = axis * axis.dot(r.direction); // Parallel component along axis
//         Vec3 d = r.direction - w;              // Perpendicular component to axis
//         Vec3 v = oc - axis * axis.dot(oc);     // Projected origin onto the plane

//         double a = d.dot(d);
//         double half_b = v.dot(d);
//         double c = v.dot(v) - radius * radius;
//         double discriminant = half_b * half_b - a * c;

//         if (discriminant > 0)
//         {
//             double sqrt_d = std::sqrt(discriminant);
//             double t = (-half_b - sqrt_d) / a;

//             if (t < t_min || t > t_max)
//             {
//                 t = (-half_b + sqrt_d) / a;
//                 if (t < t_min || t > t_max)
//                     return false;
//             }

//             Vec3 hit_point = r.at(t);
//             double projection = (hit_point - center).dot(axis);

//             if (projection >= 0 && projection <= height)
//             {
//                 rec.t = t;
//                 rec.p = hit_point;
//                 rec.normal = ((hit_point - center) - axis * projection).normalized();
//                 rec.mat_ptr = mat_ptr;
//                 return true;
//             }
//         }

//         // Check intersection with the bottom cap
//         if (intersect_caps(r, t_min, t_max, rec, center, false))
//             return true;

//         // Check intersection with the top cap
//         if (intersect_caps(r, t_min, t_max, rec, center + axis * height, true))
//             return true;

//         return false;
//     }

//     virtual bool bounding_box(double t0, double t1, aabb &output_box) const override
//     {
//         Vec3 top_center = center + axis * height;

//         // Calculate min and max bounds by expanding along cylinder's radius
//         Vec3 min_bound(
//             std::fmin(center.x - radius, top_center.x - radius),
//             std::fmin(center.y - radius, top_center.y - radius),
//             std::fmin(center.z - radius, top_center.z - radius));
//         Vec3 max_bound(
//             std::fmax(center.x + radius, top_center.x + radius),
//             std::fmax(center.y + radius, top_center.y + radius),
//             std::fmax(center.z + radius, top_center.z + radius));

//         output_box = aabb(min_bound, max_bound);
//         return true;
//     }

//     bool intersect_caps(const Ray &r, double t_min, double t_max, Hit_record &rec, const Vec3 &cap_center, bool is_top) const
//     {

//         // Project ray direction onto the cylinder's axis to find intersection with cap plane
//         double denom = r.direction.dot(axis);
//         if (std::fabs(denom) > 1e-6)
//         { // Avoid division by zero, ray parallel to cap plane
//             double t = (cap_center - r.origin).dot(axis) / denom;
//             if (t < t_min || t > t_max)
//                 return false;

//             Vec3 point = r.at(t);
//             double dist_from_center = (point - cap_center).length();

//             if (dist_from_center <= radius)
//             {
//                 rec.t = t;
//                 rec.p = point;
//                 rec.normal = is_top ? axis : -axis; // Normal points outwards from the cap
//                 rec.mat_ptr = mat_ptr;
//                 return true;
//             }
//         }
//         return false;
//     }
// };
