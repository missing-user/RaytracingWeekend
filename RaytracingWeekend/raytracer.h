#pragma once

#include <thread>
#include <chrono>

#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "color.h"
#include "sampler.h"
#include "spectrum.h"

struct tile {
    const int x, y, x_end, y_end;
    tile(int x0, int y0, int width, int height) : x(x0), y(y0), x_end(x0 + width), y_end(y0 + height) {};
    tile() : x(0), y(0), x_end(0), y_end(0) {};
};

static color ray_color(const ray& r, const hittable& h, int depth) {
    hit_record rec;
    if (depth <= 0)
        return color();

    
    if (h.hit(r, global_t_min, infinity, rec)) {
        ray scattered;
        color attenuation;
        color emitted = rec.mat_ptr->emitted(r, rec);

        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)){
            #ifdef DEBUG_DEPTH
            attenuation = color(1, 1, 1);
            #endif
            return emitted + ray_color(scattered, h, depth - 1) * attenuation;
        }
        #ifndef DEBUG_DEPTH
        return emitted;
        #endif
    }

    #ifdef DEBUG_DEPTH
    return color(depth, depth, depth);
    #endif

    //return color();
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return ( (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0)); // sky color
}

static void render_tile(vector<color>& output, const hittable& world, const unsigned int sample_count, const int max_depth, const camera& cam, const tile tile) {
    //for rendering a single tile on a thread
    for (int i = tile.x_end - 1; i >= tile.x; --i)
    {
        for (int j = tile.y_end - 1; j >= tile.y; --j)
        {
            color pixel_color = color();
            double total_weight = 0.;

            for (unsigned int s = 0; s < sample_count; ++s)
            {
                vec3 sample = sample_pixel(i, j, cam.image_width, cam.image_height, s);
                total_weight += sample.z();

                ray r = cam.get_ray(sample.x(), sample.y());

#ifdef DISPERSION
                r.wavelength = rng.random_double(lambda_start, lambda_end);
#endif // DISPERSION

                color sample_color = color();
#ifdef DEBUG_DEPTH
                sample_color = color(1, 1, 1) - (ray_color(r, world, max_depth) / max_depth);
#else
                sample_color = ray_color(r, world, max_depth) * sample.z();
#endif // DEBUG_DEPTH

#ifdef DISPERSION
                sample_color = sample_color * lambda_to_rgb(r.wavelength) * plancks_law(r.wavelength, 7200) / plancks_law(white_wavelength, 5500);
#endif // DISPERSION

                pixel_color += sample_color;
            }
            output[j * cam.image_width + i] = (pixel_color / total_weight);
        }
    }
}

static void consume_tiles(vector<color>& output, const hittable& world, int sample_count, int max_depth, const camera& cam, const vector<tile>& tiles, std::atomic_int& tile_id, std::atomic_int& finished_threads) {
    while (true) {
        if (tile_id >= tiles.size())
            break; //the queue is empty/tile is invalid, exit the thread
        const tile next = tiles[tile_id++];
        render_tile(output, world, sample_count, max_depth, cam, next);
    }
    ++finished_threads;
}

class threaded_renderer {

private:
    void create_tiles() {
        const int x_tiles = width / tile_size;
        const int y_tiles = height / tile_size;
        tiles.reserve((x_tiles + 1) * (y_tiles + 1));

        for (int tx = 0; tx < x_tiles; ++tx)
        {
            for (int ty = 0; ty < y_tiles; ++ty)
            {
                tiles.push_back(tile(tx * tile_size, ty * tile_size, tile_size, tile_size));
            }
            //create the cut off remainder tiles on the bottom
            tiles.push_back(tile(tx * tile_size, y_tiles * tile_size, tile_size, height % tile_size));
        }
        //create the cut off remainder tiles on the right
        for (int ty = 0; ty < y_tiles; ++ty)
        {
            tiles.push_back(tile(x_tiles * tile_size, ty * tile_size, width % tile_size, tile_size));
        }
        //create the tiny remainder tile in the bottom right corner
        tiles.push_back(tile(x_tiles * tile_size, y_tiles * tile_size, width % tile_size, height % tile_size));
    }

public:
    threaded_renderer(const int width, const int height, const int tile_size = 32, int sample_count = 100, int max_depth = 50) :
        width(width), height(height), tile_size(tile_size),
        sample_count(sample_count), max_depth(max_depth),
        num_threads(std::thread::hardware_concurrency())
    {}

    double get_percentage() const {
        return tile_id / static_cast<double>(tiles.size());
    }

    void render(hittable& world, camera& cam) {
        std::cerr << "Starting render with " << sample_count << " samples and " << max_depth << " bounces at " << width << "x" << height << std::endl;
        create_tiles(); // these are the jobs for the thread pool

        // create the threads for our pool, each one will independently take tiles from the queue and render them one by one until the queue is empty
        threads.reserve(num_threads);

        for (int i = 0; i < std::min(num_threads, (int)tiles.size()); ++i)
        {
            threads.push_back(
                std::thread(
                    consume_tiles,
                    ref(pixels), 
                    ref(world), 
                    sample_count, 
                    max_depth, 
                    ref(cam),
                    ref(tiles),
                    ref(tile_id),
                    ref(finished_threads))
            );
            //threads[i].detach();
        }
        std::cerr << "Created " << threads.size() << " rendering threads" << std::endl;
    }

    bool finished() const {
        return finished_threads >= num_threads;
    }
public:
    const int width, height;
    const int num_threads;
    int tile_size, sample_count, max_depth;
    vector<color> pixels{ width * height };
private:
    vector<std::thread> threads;
    vector<tile> tiles;
    std::atomic_int tile_id = 0;
    std::atomic_int finished_threads = 0;
};