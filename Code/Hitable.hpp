#pragma once
#include "aabb.hpp"

class Material;

class Hit_record
{
public:
  Vec3 p;
  Vec3 normal;
  shared_ptr<Material> mat_ptr;
  double t;
  bool front_face;

  inline void set_face_normal(const Ray &r, const Vec3 &outward_normal)
  {
    front_face = r.direction.dot(outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class Hittable
{
public:
  virtual bool hit(const Ray &r, double t_min, double t_max, Hit_record &rec) const = 0;
  virtual bool bounding_box(double t0, double t1, aabb &output_box) const = 0;

public:
  Vec3 center = Vec3(0, 0, 0);
};