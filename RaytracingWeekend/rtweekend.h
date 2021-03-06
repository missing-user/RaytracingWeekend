#pragma once

#include <memory>
#include <random>
#include "pcg_random.hpp"
#include "pcg_extras.hpp"
#include "pcg_uint128.hpp"


//#define DEBUG_DEPTH
#define DISPERSION

// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
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

//pcg random
static pcg32_fast pcg_generator;
const auto precompute_num = 65729;
double random_values[precompute_num];
int random_index = 0;

//book random
static std::mt19937 generator;
static std::uniform_real_distribution<double> distribution;

void init_random() {
    pcg_extras::seed_seq_from<std::random_device> seed_source;
    for (size_t i = 0; i < precompute_num; i++)
    {
        random_values[i] = distribution(pcg_generator);
    }
}

inline double random_double() {
    /*
    return distribution(pcg_generator); //84s
    return distribution(generator); //67s vs original 70s when creating a new distribution every frame
    return random_values[rng(precompute_num)]; //75s
    */
    return random_values[random_index++ % precompute_num]; //29s
}

inline double random_double(const double min, const double max) {
    //std::uniform_real_distribution<double> range_distribution(min, max);
    //return range_distribution(pcg_generator);
    return random_double() * (max - min) + min;
}
// Common Headers

#include "ray.h"
#include "vec3.h"