#include <iostream>
#include <fstream>
#include <thread>
#include <string>

#include "scene_generation.h"
#include "sphere.h"
#include "color.h"
#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "RaytracingWeekend.h"


color ray_color(const ray& r, const hittable& h, int depth) {
    hit_record rec;
    if (depth <= 0)
        return color();

    if (h.hit(r, 0.0001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if(rec.mat_ptr -> scatter(r, rec, attenuation, scattered))
            return  attenuation*ray_color(scattered, h, depth-1);
        return color();
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void threaded_render(std::vector<color>& output, hittable_list& world, int sample_count, int max_depth, camera& cam) {
    for (int j = cam.image_height - 1; j >= 0; --j)
    {
        std::cerr << "\rRemaining lines: " << j << ' ' << std::flush;
        for (int i = 0; i < cam.image_width; ++i)
        {
            color pixel_color = color();

            for (int s = 0; s < sample_count; ++s)
            {
                double u = (double(i) + random_double()) / (cam.image_width - 1.);
                double v = (double(j) + random_double()) / (cam.image_height - 1.);

                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            output.push_back(pixel_color / sample_count);
        }
    }
}

int main()
{

    auto start = std::chrono::high_resolution_clock::now();
    //Camera Settings
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    //Render Settings
    const std::string filename = "render.ppm";
    int sample_count = 100;
    int max_depth = 50;
    const int num_threads = 16;

    // World
    hittable_list world = random_scene();

    //Render
    auto output_file = std::ofstream(filename);
    if (!output_file) {
        std::cerr << "Cannot open file.\n";
        return 1;
    }
    output_file<< "P3\n" << cam.image_width << ' ' << cam.image_height << "\n255\n";

    std::vector<std::thread> threads;
    std::vector<color> pixels[num_threads];

    for (int i = 0; i < num_threads; ++i)
    {
        std::cerr << "Started thread: "<<i+1<<'\n';
        threads.push_back(std::thread(threaded_render, std::ref(pixels[i]), std::ref(world), sample_count / num_threads, max_depth, std::ref(cam)));
        //threaded_render(pixels[i], world, sample_count/num_threads, max_depth, cam);
    }

    for (int i = 0; i < num_threads; ++i)
    {
       threads[i].join();
    }
    const auto elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cerr << "Render took: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << "ms " << std::endl;

    std::cerr << "\nCollecting threads\n";
    for (int j = 0; j < cam.image_height*cam.image_width; ++j)
    {
        color tmp_color = color();
        for (int i = 0; i < num_threads; ++i)
        {
            tmp_color += pixels[i][j];
        }
        write_color(output_file, tmp_color / num_threads);
    }

    const auto elapsed2 = std::chrono::high_resolution_clock::now() - start;
    std::cerr << "Render + file write took: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed2).count() << "ms " << std::endl;
        
    std::cerr << "\nDone";
}