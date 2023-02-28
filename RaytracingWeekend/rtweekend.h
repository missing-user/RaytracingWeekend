#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

typedef glm::highp_dvec3 vec3;
typedef vec3 point3;

#include <random>
#include <memory>
#include "pcg_extras.hpp"
#include "pcg_random.hpp"
#include "pcg_uint128.hpp"

//#define EXR_SUPPORT
//#define DISPERSION
#define LAMBERT_BEER

static thread_local std::mt19937 twister{};
static thread_local pcg32_fast pcgrng{};
#define RANDOM twister //select the active random

// random distribution
static std::uniform_real_distribution<double> dis(0.0, 1.0);
static std::normal_distribution<double> normal_dis(0., .2);

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
inline double clamp(const double x, const double min=0, const double max=1) {
    if (x > max)
        return max;
    else if (x < min)
        return min;
    return x;
}


inline double random_double() {
    return dis(RANDOM);
}

int random_int(const int min, const int max) {
    return (RANDOM() % (max - min)) + min;
}

double random_normal_double() {

    return normal_dis(RANDOM);
    //return prng.rand_normal();
}

double random_double(const double min, const double max) {
    return random_double() * (max - min) + min;
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
        if (glm::length2(p) < 1)
            return p;
    }
}

vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (glm::length2(p) < 1)
            return p;
    }
}

vec3 random_unit_vector() {
    return glm::normalize(random_in_unit_sphere());
}

#include "spectrum.h"
#include "ray.h"