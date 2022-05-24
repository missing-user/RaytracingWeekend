#pragma once

#include <thread>

#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "color.h"


struct tile {
    const int x, y, x_end, y_end;
    tile(int x0, int y0, int width, int height) : x(x0), y(y0), x_end(x0 + width), y_end(y0 + height) {};
    tile() : x(0), y(0), x_end(0), y_end(0) {};
};

static color ray_color(const ray& r, const hittable& h, int depth) {
    hit_record rec;
    if (depth <= 0)
        return color();

    if (h.hit(r, 0.0001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return  attenuation * ray_color(scattered, h, depth - 1);
        return color();
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

static void render_tile(std::vector<color>& output, hittable& world, int sample_count, int max_depth, camera& cam, const tile tile) {
    //for rendering a single tile on a thread
    for (int j = tile.y_end - 1; j >= tile.y; --j)
    {
        for (int i = tile.x; i < tile.x_end; ++i)
        {
            color pixel_color = color();

            for (int s = 0; s < sample_count; ++s)
            {
                double u = (i + random_double()) / (cam.image_width - 1.);
                double v = (j + random_double()) / (cam.image_height - 1.);

                ray r = cam.get_ray(u, v);
                //pixel_color += ray_color(r, world, max_depth);

                for (int lambda = 0; lambda < 34; lambda++)
                {
                    //r.wavelength = random_double(380, 720);
                    r.wavelength = 380 + lambda * 10;
                    pixel_color += ray_color(r, world, max_depth) * wavelength_to_color(r.wavelength)/8;
                }
            }
            output[j * cam.image_width + i] += (pixel_color / sample_count);
        }
    }
}

static void consume_tiles(std::vector<color>& output, hittable& world, int sample_count, int max_depth, camera& cam, std::vector<tile>& tiles, std::atomic_int& tile_id, std::atomic_int& finished_threads) {
    // This loop is running on every thread and consumes a new tile each time it finishes its previous one, until the atomic counter is larger than the number of tiles
    while (true) {
        if (tile_id >= tiles.size())
            break; //the queue is empty/tile is invalid, exit the thread
        tile next = tiles[tile_id++];
        render_tile(output, world, sample_count, max_depth, cam, next);
    }
    ++finished_threads;
}

class threaded_renderer {

private:
    void create_tiles() {
        int x_tiles = floor(width / tile_size);
        int y_tiles = floor(height / tile_size);

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
    threaded_renderer(int width, int height, int tile_size = 32, int sample_count = 100, int max_depth = 50) :
        width(width), height(height), tile_size(tile_size),
        sample_count(sample_count), max_depth(max_depth),
        num_threads(std::thread::hardware_concurrency())
    {}

    void render(hittable& world, camera& cam) {
        create_tiles(); // these are the jobs for the thread pool

        // create the threads for our pool, each one will independently take tiles from the queue and render them one by one until the queue is empty
        threads.reserve(num_threads);

        for (int i = 0; i < std::min(num_threads, (int)tiles.size()); ++i)
        {
            threads.push_back(
                std::thread(
                    consume_tiles,
                    std::ref(pixels), 
                    std::ref(world), 
                    sample_count, 
                    max_depth, 
                    std::ref(cam),
                    std::ref(tiles),
                    std::ref(tile_id),
                    std::ref(finished_threads))
            );
            //threads[i].detach();
        }
        std::cerr << "Started " << threads.size() << " rendering threads" << std::endl;
    }

    bool finished() const {
        return finished_threads >= num_threads;
    }
public:
    const int width, height;
    const int num_threads;
    int tile_size, sample_count, max_depth;
    std::vector<color> pixels{ width * height };
private:
    std::vector<std::thread> threads;
    std::vector<tile> tiles;
    std::atomic_int tile_id = 0;
    std::atomic_int finished_threads = 0;
};