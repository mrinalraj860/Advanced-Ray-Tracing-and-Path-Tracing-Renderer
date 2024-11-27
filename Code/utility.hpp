#pragma once
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <chrono>

using std::make_shared;
using std::shared_ptr;
using std::sqrt;
using namespace std::chrono;

const double inf = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180;
}

inline double random_double()
{
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}

inline double clamp(double x, double min, double max)
{
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

double schlick(double cosine, double ref_idx)
{
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

inline int random_int(int min, int max)
{
    return static_cast<int>(random_double(min, max + 1));
}

#include "Ray.hpp"
#include "vec3.hpp"
