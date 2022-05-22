#include <iostream>
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

void threaded_render(std::string& output, hittable_list& world, int sample_count, int max_depth, camera& cam, int start_row, int end_row) {
    for (int j = end_row - 1; j >= start_row; --j)
    {
        //std::cerr << "\rScanlines remaining: " << cam.image_height - (start_row- end_row - j)  << ' ' << std::flush;
        for (int i = 0; i < cam.image_width; ++i)
        {
            color pixel_color = color(0., 0., 0.);

            for (int s = 0; s < sample_count; ++s)
            {
                double u = (double(i) + random_double()) / (cam.image_width - 1.);
                double v = (double(j) + random_double()) / (cam.image_height - 1.);

                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }

            write_color(output, pixel_color, sample_count);
        }
    }
}

int main()
{
    //Camera Settings
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    //Render Settings
    int sample_count = 500;
    int max_depth = 80;
    const int num_threads = 8;

    // World
    hittable_list world = random_scene();

    //Render
    std::cout << "P3\n" << cam.image_width << ' ' << cam.image_height << "\n255\n";

    int remaining_rows = 0;
    int step = static_cast<int>(cam.image_height / num_threads);
    std::vector<std::thread> threads;
    std::vector<std::string> outputs(num_threads, "");

    for (int i = 0; i < num_threads-1; ++i)
    {
        //threads.push_back(std::thread(threaded_render, std::ref(outputs[i]), std::ref(world), sample_count, max_depth, std::ref(cam), step * i, step * (i + 1)));
        threaded_render(outputs[i], world, sample_count, max_depth, cam, step* i, step* (i + 1));
        remaining_rows += step;
    }
    //threads.push_back(std::thread(threaded_render, std::ref(outputs[num_threads - 1]), std::ref(world), sample_count, max_depth, std::ref(cam), remaining_rows, cam.image_height));
    threaded_render(outputs[num_threads - 1], world, sample_count, max_depth, cam, remaining_rows, cam.image_height);

    for (int i = num_threads - 1; i >= 0; --i)
    {
        threads[i].join();
        std::cout << outputs[i];
    }

        
    std::cerr << "\nDone";
}