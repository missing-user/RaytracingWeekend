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
#include "variance_welford.h"

struct tile
{
    const int x, y, x_end, y_end;
    tile(int x0, int y0, int width, int height) : x(x0), y(y0), x_end(x0 + width), y_end(y0 + height){};
    tile() : x(0), y(0), x_end(0), y_end(0){};
};

color ray_color(const ray &r, const hittable &h, int depth, normal3 &normal)
{
    color result{0, 0, 0};
    vec3 attenuation{1, 1, 1};
    normal = {0, -1, 0}; // set normal to a sensible default for rays that didn't hit anything
    ray current_ray = r;

    bool hitDiffuse = false;
    for (int i = 0; i < depth; i++)
    {
        hit_record rec;

        if (h.hit(current_ray, global_t_min, infinity, rec))
        {
            // We hit an object, update color based on emission and attenuation
            color emitted = rec.mat_ptr->emitted(current_ray, rec);
            ray scattered;

            // Store the normal of the first diffuse/opaque ray hit
            if (!hitDiffuse && (dynamic_cast<dielectric *>(rec.mat_ptr) == nullptr) && (dynamic_cast<thinfilm *>(rec.mat_ptr) == nullptr))
            {
                normal = rec.normal;
                hitDiffuse = true;
            }

            if (rec.mat_ptr->scatter(current_ray, rec, attenuation, scattered))
            {
                result += emitted * attenuation;
                current_ray = scattered;
            }
            else
            {
                return result + emitted * attenuation;
            }
        }
        else
        {
            // Background color / sky sphere
            // return color(0, 0, 0); // black sky
            vec3 unit_direction = glm::normalize(current_ray.direction());
            auto t = 0.5 * (unit_direction.y + 1.0);
            result += attenuation * ((1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0));
            return result;
        }
    }

    // Exceeded ray depth
    return result = {0, 0, 0};
}

// Function to calculate maximum variance
color calculate_max_variance(vector<weighted_variance_welford<color>>& pixel_colors) {
    color max_var = {0, 0, 0};
    for(auto& pixel_color : pixel_colors)
    {
        max_var = glm::max(max_var, pixel_color.variance());
    }
    return max_var;
}

// Function to override variance
void override_variance(vector<weighted_variance_welford<color>>& pixel_colors, color max_var) {
    for(auto& pixel_color : pixel_colors)
    {
        pixel_color.override_variance(max_var);
    }
}

void render_tile(vector<color> &output, vector<normal3> &output_normal, const hittable &world, const std::size_t sample_count, const int max_depth, const camera &cam, const tile tile)
{
    // for rendering a single tile on a thread
    vector<weighted_variance_welford<color>> pixel_colors;
    pixel_colors.resize((tile.x_end - tile.x) * (tile.y_end - tile.y));
    int sample_batch_size = sample_count/20;

    for (int i = tile.x_end - 1; i >= tile.x; --i)
    {
        for (int j = tile.y_end - 1; j >= tile.y; --j)
        {
            auto &pixel_color = pixel_colors[(j - tile.y) * (tile.x_end - tile.x) + (i - tile.x)];
            std::size_t s;
            for (s = 1; s <= sample_count; ++s)
            {
                PixelSample sample = sample_pixel(i, j, cam.image_width, cam.image_height, s);
                ray r = cam.get_ray(sample.u, sample.v);
#ifdef DISPERSION
                auto lambda_weight_pair = random_wavelength(s, sample_count);
                r = {r, lambda_weight_pair.first}; // Apply the wavelength to a ray
#endif                                             // DISPERSION
                color sample_color = ray_color(r, world, max_depth, output_normal[j * cam.image_width + i]);

#ifdef DISPERSION
                sample_color *= lambda_to_rgb(r.lambda());
                sample.weight *= lambda_weight_pair.second; // Weight for the wavelength sample
#endif                                                      // DISPERSION
                pixel_color.add_sample(sample_color, sample.weight);


                if(s%sample_batch_size == 0)
                {
                    // Locally average the variance of neighboring pixels
                    // override_variance(pixel_colors, calculate_max_variance(pixel_colors));
                    if( luminance(pixel_color.convergence() / pixel_color.mean()) < 2./std::sqrt(sample_count))
                    {
                        // Early exit if the pixel is converged
                        break;
                    }
                }
            }
            output[j * cam.image_width + i] = pixel_color.mean();//color(static_cast<double>(s)/sample_count)
        }
    }
}

void consume_tiles(vector<color> &output, vector<normal3> &output_normal, const hittable &world, int sample_count, int max_depth, const camera &cam, const vector<tile> &tiles, std::atomic_int &tile_id, std::atomic_int &finished_threads)
{
    while (tile_id < tiles.size())
    { // the queue is empty/tile is invalid, exit the thread
        const tile next = tiles[tile_id++];
        render_tile(output, output_normal, world, sample_count, max_depth, cam, next);
    }
    ++finished_threads;
}

class threaded_renderer
{

private:
    void create_tiles()
    {
        const int x_tiles = width / tile_size;
        const int y_tiles = height / tile_size;
        tiles.clear();
        tiles.reserve((x_tiles + 1) * (y_tiles + 1));

        for (int tx = 0; tx < x_tiles; ++tx)
        {
            for (int ty = 0; ty < y_tiles; ++ty)
            {
                tiles.push_back(tile(tx * tile_size, ty * tile_size, tile_size, tile_size));
            }
            // create the cut off remainder tiles on the bottom
            tiles.push_back(tile(tx * tile_size, y_tiles * tile_size, tile_size, height % tile_size));
        }
        // create the cut off remainder tiles on the right
        for (int ty = 0; ty < y_tiles; ++ty)
        {
            tiles.push_back(tile(x_tiles * tile_size, ty * tile_size, width % tile_size, tile_size));
        }
        // create the tiny remainder tile in the bottom right corner
        tiles.push_back(tile(x_tiles * tile_size, y_tiles * tile_size, width % tile_size, height % tile_size));
    }

public:
    threaded_renderer(const int width, const int height, const int tile_size = 32, int sample_count = 100, int max_depth = 50) : width(width), height(height),
                                                                                                                                 pixels({static_cast<size_t>(width * height)}),
                                                                                                                                 pixels_normal({static_cast<size_t>(width * height)}),
                                                                                                                                 tile_size(tile_size),
                                                                                                                                 sample_count(sample_count), max_depth(max_depth),
                                                                                                                                 num_threads(std::thread::hardware_concurrency())
    {
        create_tiles(); // these are the jobs for the thread pool
    }

    double get_percentage() const
    {
        return tile_id / static_cast<double>(tiles.size());
    }

    void stop_render()
    {
        for (auto &t : threads)
            t.join();
        threads.clear();
        finished_threads = 0;
        tile_id = 0;

        // Show the previous frame grayed out
        std::transform(pixels.begin(), pixels.end(), pixels.begin(), [](auto &c)
                       { return c * 0.33; });
        // std::fill(pixels.begin(), pixels.end(), color(0, 0, 0));
    }

    void render(hittable &world, camera &cam)
    {
        stop_render();
        std::cerr << "Starting render with " << sample_count << " samples and " << max_depth << " bounces at " << width << "x" << height << std::endl;

        // create the threads for our pool, each one will independently take tiles from the queue and render them one by one until the queue is empty
        threads.resize(num_threads);

        for (int i = 0; i < std::min(num_threads, (int)tiles.size()); ++i)
        {
            threads[i] = std::thread(
                consume_tiles,
                ref(pixels),
                ref(pixels_normal),
                ref(world),
                sample_count,
                max_depth,
                ref(cam),
                ref(tiles),
                ref(tile_id),
                ref(finished_threads));
            // threads[i].detach();
        }
        std::cerr << "Created " << threads.size() << " rendering threads\n";
    }

    bool finished() const
    {
        return finished_threads >= num_threads;
    }

public:
    const int width, height;
    const int num_threads;
    const int tile_size, sample_count, max_depth;
    vector<color> pixels;
    vector<normal3> pixels_normal;

private:
    vector<std::thread> threads;
    vector<tile> tiles;
    std::atomic_int tile_id = 0;
    std::atomic_int finished_threads = 0;
};