#pragma once

#include <memory>
#include "prng.h"
#include "vec3.h"
#include "ray.h"
#include "pcg_extras.hpp"
#include "pcg_random.hpp"
#include "pcg_uint128.hpp"

//#define EXR_SUPPORT
//#define DEBUG_DEPTH
//#define DISPERSION


static thread_local smallprng::knuth_lcg prng;
static thread_local std::mt19937 twister;
static thread_local pcg32_fast pcgrng;

// random distribution
static std::uniform_real_distribution<double> dis(0.0, 1.0);

#define RANDOM_32BIT static_cast<double>(prng.randf())
#define RANDOM_64BIT prng.rand()
#define RANDOM_TWISTER dis(twister)
#define RANDOM_PCG dis(pcgrng)

#define RANDOM RANDOM_64BIT //select the active random

// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using std::ref;
using std::vector;

// Constants
const double global_t_min = 1e-6; //DBL_EPSILON?
const double infinity = DBL_MAX;
const double pi = 3.1415926535897932385;
const double aspect_ratio = 16.0 / 9.0;

// Utility Functions
inline double degrees_to_radians(const double degrees) {
    return degrees * pi / 180.0;
}

inline double clamp(const double x, const double min=0, const double max=1) {
    if (x > max)
        return max;
    else if (x < min)
        return min;
    return x;
}

inline double random_double() {
    return RANDOM;
}


static int random_int(const int min, const int max) {
    auto tmpprng = smallprng::knuth_lcg();
    return (tmpprng() % (max - min)) + min;
}

inline double random_normal_double() {
    return prng.rand_normal(0., .2);
    //return prng.rand_normal();
}

inline double random_double(const double min, const double max) {
    return random_double() * (max - min) + min;
}

inline vec3 random() {
    return vec3(random_double(), random_double(), random_double());
}

inline vec3 random(double min, double max) {
    return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

inline vec3 random_in_unit_sphere() {
    while (true) {
        auto p = random(-1, 1);
        if (p.length() < 1)
            return p;
    }
}

inline vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length() < 1)
            return p;
    }
}

inline vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}