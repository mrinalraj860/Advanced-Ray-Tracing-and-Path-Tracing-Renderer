#pragma once
#include "Hitable.hpp"
#include "utility.hpp"
#include "Material.hpp"

class Sphere : public Hittable
{

public:
    Vec3 center;
    float radius;
    shared_ptr<Material> mat_ptr;
    Sphere() {}
    Sphere(const Vec3 &cen, float r, std::shared_ptr<Material> &mat)
        : center(cen), radius(r), mat_ptr(mat) {}

    bool hit(const Ray &r, double t_min, double t_max, Hit_record &rec) const override
    {
        Vec3 oc = r.origin - center;
        auto a = r.direction.length_squared();
        auto half_b = oc.dot(r.direction);
        auto c = oc.length_squared() - radius * radius;
        auto discriminant = half_b * half_b - a * c;

        if (discriminant > 0)
        {
            auto sqrt_d = sqrt(discriminant);

            // Find the nearest root that lies in the acceptable range.
            auto root = (-half_b - sqrt_d) / a;
            if (root < t_min || root > t_max)
            {
                root = (-half_b + sqrt_d) / a;
                if (root < t_min || root > t_max)
                    return false;
            }

            rec.t = root;
            rec.p = r.at(rec.t);
            Vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat_ptr = mat_ptr;
            return true;
        }
        return false;
    }

    // bool intersect(const Ray &r, float t_min, float t_max, Hit_record &rec) const
    // {
    //     Vec3 oc = r.origin - center;
    //     float a = r.direction.length_squared();
    //     float b = 2.0f * oc.dot(r.direction);
    //     float c = oc.length_squared() - radius * radius;
    //     float discriminant = b * b - 4 * a * c;

    //     if (discriminant > 0)
    //     {
    //         auto root = sqrt(discriminant);
    //         auto temp = (-b - root) / (2.0f * a);
    //         if (temp < t_max && temp > t_min)
    //         {
    //             rec.t = temp;
    //             rec.p = r.at(rec.t);
    //             Vec3 outward_normal = (rec.p - center) / radius;
    //             rec.set_face_normal(r, outward_normal);
    //             rec.mat_ptr = mat_ptr;
    //             return true;
    //         }
    //         temp = (-b + root) / a;
    //         if (temp < t_max && temp > t_min)
    //         {
    //             rec.t = temp;
    //             rec.p = r.at(rec.t);
    //             Vec3 outward_normal = (rec.p - center) / radius;
    //             rec.set_face_normal(r, outward_normal);
    //             rec.mat_ptr = mat_ptr;
    //             return true;
    //         }
    //     }

    //     return false;
    // }

    bool bounding_box(double t0, double t1, aabb &output_box) const
    {
        output_box = aabb(center - Vec3(radius, radius, radius),
                          center + Vec3(radius, radius, radius));
        return true;
    }
};
