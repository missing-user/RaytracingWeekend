#pragma once

#include <thread>
#include <chrono>

#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "color.h"

const bool DEBUG_DEPTH = false;
const bool DISPERSION = true;

struct tile {
    const int x, y, x_end, y_end;
    tile(int x0, int y0, int width, int height) : x(x0), y(y0), x_end(x0 + width), y_end(y0 + height) {};
    tile() : x(0), y(0), x_end(0), y_end(0) {};
};

static color dispersed_ray_color(const hit_record& rec, const ray& r, const hittable& h, int depth);

static color ray_color(const ray& r, const hittable& h, int depth) {
    hit_record rec;
    if (depth <= 0)
        return color();

    if (h.hit(r, 0.0001, infinity, rec)) {
        ray scattered;
        color attenuation;
        color emitted = rec.mat_ptr->emitted(rec.p);

        if (DISPERSION && std::dynamic_pointer_cast<dielectric>(rec.mat_ptr) && r.lambda() == white_wavelength) {
            return  emitted + dispersed_ray_color(rec, r, h, depth-1);
        }else if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)){
            if (DEBUG_DEPTH)
                attenuation = color(1, 1, 1);
            return emitted + ray_color(scattered, h, depth - 1) * attenuation;
        }
        return DEBUG_DEPTH ? color(depth, depth, depth): emitted;
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    if (DEBUG_DEPTH)
        return color(depth, depth, depth);
    else
        //return color();
        return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

static color dispersed_ray_color(const hit_record& rec, const ray& r, const hittable& h, int depth) {
    color tmp_ray_col = color();
    for (int lambda = 0; lambda < 3; lambda++)
    {
        ray dispersed_ray = ray(r, 380. + lambda * 340. / 3.);
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(dispersed_ray, rec, attenuation, scattered)) {
            color disp_color = color(0, 0, 1);
            if (lambda == 1)
                disp_color = color(0, 1, 0);
            else if (lambda == 2)
                disp_color = color(1, 0, 0);

            if (DEBUG_DEPTH)
                attenuation = color(1, 1, 1);

            tmp_ray_col += ray_color(scattered, h, depth - 1) * attenuation * disp_color;
        }
    }

    return tmp_ray_col;
}

static void render_tile(std::vector<color>& output, hittable& world, const int sample_count, const int max_depth, camera& cam, const tile tile) {
    //for rendering a single tile on a thread
    for (int i = tile.x_end - 1; i >= tile.x; --i)
    {
        for (int j = tile.y_end - 1; j >= tile.y; --j)
        {
            color pixel_color = color();

            for (int s = 0; s < sample_count; ++s)
            {
                double u = (i + random_double()) / (cam.image_width - 1.);
                double v = (j + random_double()) / (cam.image_height - 1.);

                ray r = cam.get_ray(u, v);
                if(DEBUG_DEPTH)
                    pixel_color += color(1, 1, 1) - (ray_color(r, world, max_depth) / max_depth);
                else
                    pixel_color += ray_color(r, world, max_depth);
            }
            output[j * cam.image_width + i] = (pixel_color / sample_count);
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