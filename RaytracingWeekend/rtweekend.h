#pragma once

#include <memory>
#include <random>


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

inline double random_double(const double min = 0, const double max = 1) {
    static std::uniform_real_distribution<double> distribution(min, max);
    static std::mt19937 generator;
    return distribution(generator);
}

// Common Headers

#include "ray.h"
#include "vec3.h"