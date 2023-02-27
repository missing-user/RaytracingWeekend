#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>

#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "color.h"
#include "sampler.h"
#include "spectrum.h"

__device__ color ray_color(curandState* rng, const ray& r, hittable** h, int depth, glm::fvec3& normal) {
    color result{ 0, 0, 0 };
    vec3 attenuation{ 1, 1, 1 };
    normal = { 0,-1,0 }; // set normal to a sensible default for rays that didn't hit anything
    ray current_ray = r;

    bool hitDiffuse = false;
    for (int i = 0; i < depth; i++) {
        hit_record rec;

        if ((*h)->hit(current_ray, global_t_min, infinity, rec)) {
            // We hit an object, update color based on emission and attenuation
            color emitted = rec.mat_ptr->emitted(current_ray, rec);
            ray scattered;

            // Store the normal of the first diffuse/opaque ray hit
            if (!hitDiffuse) {
                normal = rec.normal;
                hitDiffuse = true;
            }

            if (rec.mat_ptr->scatter(rng, current_ray, rec, attenuation, scattered)) {
                result += emitted * attenuation;
                current_ray = scattered;
            } else {
                return result + emitted * attenuation;
            }
        } else {
            // Background color / sky sphere
            vec3 unit_direction = glm::normalize(current_ray.direction());
            auto t = 0.5 * (unit_direction.y + 1.0);
            result += attenuation * ((1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0));
            return result;
        }
    }

    // Exceeded ray depth
    return { 0,0,0 };
}

__global__ void random_init(int max_x, int max_y, curandState* rand_state) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    if ((i >= max_x) || (j >= max_y)) return;
    int pixel_index = j * max_x + i;
    //Each thread gets same seed, a different sequence number, no offset
    curand_init(1984, pixel_index, 0, &rand_state[pixel_index]);
}

__global__ void render_gpu(vec3* output, glm::fvec3* output_normal, int sample_count, int max_depth, camera** cam, hittable** world, curandState* rand_state) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    if ((i >= (*cam)->image_width) || (j >= (*cam)->image_height)) return;
    int pixel_index = j * (*cam)->image_width + i;
    curandState local_rand_state = rand_state[pixel_index];

    vec3 pixel_color{ 0, 0, 0 };
    double total_weight = 0.;
    for (size_t s = 0; s < sample_count; s++) {

        vec3 sample = sample_pixel(&local_rand_state, i, j, (*cam)->image_width, (*cam)->image_height, s);
        total_weight += sample.z;

        ray r = (*cam)->get_ray(&local_rand_state, sample.x, sample.y);
#ifdef DISPERSION
        auto lambda_weight_pair = random_wavelength(local_rand_state, s, sample_count);
        r = { r, lambda_weight_pair.lambda };
#endif // DISPERSION
        color sample_color = ray_color(&local_rand_state, r, world, max_depth, output_normal[pixel_index]);

#ifdef DISPERSION
        sample_color *= lambda_to_rgb(r.lambda());
        sample_color *= lambda_weight_pair.probability; // Weight for the wavelength sample
#endif // DISPERSION

        pixel_color += sample_color *= sample.z; // Weight for the pixel sample position
    }
    output[pixel_index] = pixel_color / total_weight;
    rand_state[pixel_index] = local_rand_state;
}

class threaded_renderer {
public:
    threaded_renderer(const int width, const int height, const int tile_size = 8, int sample_count = 100, int max_depth = 50) :
        width(width), height(height), num_pixels(width* height),
        tile_size(tile_size),
        sample_count(sample_count), max_depth(max_depth)
    {
        checkCudaErrors(cudaMallocManaged((void**)&pixels, sizeof(color) * num_pixels));
        checkCudaErrors(cudaMallocManaged((void**)&pixels_normal, sizeof(glm::fvec3) * num_pixels));
    }

    double get_percentage() const {
        return .5;
    }

    void stop_render() {
        // Show the previous frame grayed out
        // std::transform(pixels.begin(), pixels.end(), pixels.begin(), [](auto& c) {return c * 0.33; });
        // std::fill(pixels.begin(), pixels.end(), color(0, 0, 0));
    }

    void render(hittable** world, camera** cam) {
        stop_render();

        dim3 blocks(width / tile_size + 1, height / tile_size + 1);
        dim3 threads(tile_size, tile_size);

        curandState* d_rand_state;
        checkCudaErrors(cudaMalloc((void**)&d_rand_state, num_pixels * sizeof(curandState)));
        random_init << <blocks, threads >> > (width, height, d_rand_state);
        checkCudaErrors(cudaDeviceSynchronize());
        std::cerr << "Starting render with " << sample_count << " samples and " << max_depth << " bounces at " << width << "x" << height << std::endl;
        render_gpu << <blocks, threads >> > (pixels, pixels_normal, sample_count, max_depth, cam, world, d_rand_state);
        checkCudaErrors(cudaGetLastError());
        checkCudaErrors(cudaDeviceSynchronize());
    }

    bool finished() const {
        return false;
    }
public:
    const int width, height, num_pixels;
    const int tile_size, sample_count, max_depth;
    color* pixels;
    glm::fvec3* pixels_normal;
};