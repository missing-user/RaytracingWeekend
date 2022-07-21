#pragma once

#include <memory>
#include "prng.h"
#include "vec3.h"
#include "ray.h"

#define EXR_SUPPORT
//#define DEBUG_DEPTH
#define DISPERSION
#define DISCRETE_DISPERSION

#define RANDOM_PRECALC_SHUFFLED random_values[random_index++ & (precompute_num-1)]
#define RANDOM_PRECALC_MOD random_values[prng() & (precompute_num - 1)]
#define RANDOM_32BIT static_cast<double>(prng.randf())
#define RANDOM_64BIT prng.rand()

#define RANDOM RANDOM_64BIT //select the active random

// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants
const double global_t_min = 1e-6; //DBL_EPSILON?
const double infinity = DBL_MAX;
const double pi = 3.1415926535897932385;
const double aspect_ratio = 16.0 / 9.0;
const double white_wavelength = 550.;

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

class RNG {
public:
    RNG() : random_index(0) {
        prng = smallprng::knuth_lcg();
        init_random();
    }

    static int random_int(const int min, const int max) {
        auto tmpprng = smallprng::knuth_lcg();
        return (tmpprng() % (max - min)) + min;
    }

    double random_normal_double() {
        return prng.rand_normal(0., .2);
        //return prng.rand_normal();
    }

    double random_double() {
        return RANDOM;
    }

    double random_double(const double min, const double max) {
        return RANDOM * (max - min) + min;
    }

    vec3 random() {
        return vec3(random_double(), random_double(), random_double());
    }

    vec3 random(double min, double max) {
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }

    vec3 random_in_unit_sphere() {
        while (true) {
            auto p = random(-1, 1);
            if (p.length() < 1)
                return p;
        }
    }

    vec3 random_in_unit_disk() {
        while (true) {
            auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
            if (p.length() < 1)
                return p;
        }
    }

    vec3 random_unit_vector() {
        return unit_vector(random_in_unit_sphere());
    }
private:
    void init_random() {
        for (size_t i = 0; i < precompute_num; i++)
        {
            random_values[i] = prng.rand();
        }
    }
private:
    static const unsigned int precompute_num = 1 << 12;
    double random_values[precompute_num];
    unsigned int random_index; //This is a uint so it overflows to 0 instead of a negative number. Otherwise more expensive checks would be needed
    smallprng::knuth_lcg prng;
};