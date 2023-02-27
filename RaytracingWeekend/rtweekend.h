#pragma once
#define GLM_FORCE_CUDA
#define CUDA_VERSION 12000
#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

typedef glm::dvec3 vec3;
typedef vec3 point3;

#include <cuda/std/utility>
#include <cuda/std/array>
#include <cuda/std/iterator>
#include <cuda/std/tuple>
#include <curand_kernel.h>
#include <random>
#include <memory>

//#define EXR_SUPPORT
//#define DISPERSION
#define LAMBERT_BEER

static thread_local std::mt19937 twister{};
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
__device__ const double global_t_min = 1e-6; //DBL_EPSILON?
__device__ const double infinity = DBL_MAX;
__device__ const double pi = 3.1415926535897932385;
__device__ const double aspect_ratio = 16.0 / 9.0;


// limited version of checkCudaErrors from helper_cuda.h in CUDA examples
#define checkCudaErrors(val) check_cuda( (val), #val, __FILE__, __LINE__ )

void check_cuda(cudaError_t result, char const* const func, const char* const file, int const line) {
    if (result) {
        std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " <<
            file << ":" << line << " '" << func << "' \n";
        // Make sure we call CUDA Device Reset before exiting
        cudaDeviceReset();
        exit(99);
    }
}

// Utility Functions
__host__ __device__ inline double clamp(const double x, const double min=0, const double max=1) {
    if (x > max)
        return max;
    else if (x < min)
        return min;
    return x;
}

__device__ inline double random_double(curandState* local_rand_state) {
    return curand_uniform(local_rand_state); 
}

__device__ double random_normal_double(curandState* local_rand_state) {

    return curand_normal(local_rand_state);
    //return prng.rand_normal();
}

__device__ double random_double(curandState* local_rand_state, const double min, const double max) {
    return random_double(local_rand_state) * (max - min) + min;
}

__device__ vec3 random(curandState* local_rand_state) {
    return vec3(random_double(local_rand_state), random_double(local_rand_state), random_double(local_rand_state));
}

__device__ vec3 random(curandState* local_rand_state, double min, double max) {
    return vec3(random_double(local_rand_state,min, max), random_double(local_rand_state,min, max), random_double(local_rand_state,min, max));
}

__device__ vec3 random_in_unit_sphere(curandState* local_rand_state) {

    while (true) {
        auto p = random(local_rand_state , -1, 1);
        if (glm::length2(p) < 1)
            return p;
    }
}

__device__ vec3 random_in_unit_disk(curandState* local_rand_state) {
    while (true) {
        auto p = vec3(random_double(local_rand_state ,-1, 1), random_double(local_rand_state ,-1, 1), 0);
        if (glm::length2(p) < 1)
            return p;
    }
}

__device__ vec3 random_unit_vector(curandState* rng) {
    return glm::normalize(random_in_unit_sphere(rng));
}

#include "spectrum.h"
#include "ray.h"