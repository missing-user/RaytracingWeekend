#pragma once

#include <memory>
#include <random>
#include "pcg_random.hpp"
#include "pcg_extras.hpp"
#include "pcg_uint128.hpp"


//#define DEBUG_DEPTH
//#define DISPERSION
//#define DISCRETE_DISPERSION

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

//pcg random
static pcg32_fast pcg_generator;
const int precompute_num = 16777216; //16mio random numbers should be enough... right?
double random_values[precompute_num];
std::atomic_uint random_index = 0; //This is a uint so it overflows to 0 instead of a negative number. Otherwise more expensive checks would be needed

//book random
static std::mt19937 twister_generator;
static std::uniform_real_distribution<double> distribution;
static std::normal_distribution<double> normal_distribution{0., .2}; //normal distribution approximately between -.5 and .5

void init_random() {
    pcg_extras::seed_seq_from<std::random_device> seed_source;
    for (size_t i = 0; i < precompute_num; i++)
    {
        random_values[i] = distribution(pcg_generator);
    }
}

inline double random_normal_double() {
    return normal_distribution(twister_generator);
}

inline double random_double() {
    /*
    return distribution(pcg_generator); //84s
    return distribution(twister_generator); //67s vs original 70s when creating a new distribution every frame
    return random_values[rng(precompute_num)]; //75s
    return random_values[random_index++ % precompute_num]; //29s
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