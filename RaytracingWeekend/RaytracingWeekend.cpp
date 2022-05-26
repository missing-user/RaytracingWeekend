#include <iostream>
#include <string>
#include <chrono>

#include "preview_gui.h"

#include "scene_generation.h"
#include "sphere.h"
#include "raytracer.h"
#include "color.h"
#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"

std::string get_filename(int argc, char* argv[], std::string file_ending,bool append_time) {
    std::string cmd = "out";
    for (int i = 1; i < argc; ++i)
    {
        cmd = argv[i];
    }
    cmd += file_ending;

    return cmd;
}


int main(int argc, char* argv[])
{
    auto start = std::chrono::high_resolution_clock::now();
    std::cerr << "Initializing Renderer" << std::endl;

    //Render Settings
    const std::string filename = get_filename(argc, argv, ".png", true);

    //Camera Settings
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    double dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aperture, dist_to_focus, 720);

    //Render
    threaded_renderer renderer(cam.image_width, cam.image_height, 32, 100, 50);
    preview_gui gui(filename, cam.image_width, cam.image_height);

    std::cerr << "Initializing Scene" << std::endl;

    // World
    hittable_list world = random_disp();

    std::cerr << "Starting Render" << std::endl;
    renderer.render(world, cam);
    gui.open_gui(renderer);
    const auto elapsed = std::chrono::high_resolution_clock::now() - start;

    std::cerr<<"Render took: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << "ms " << std::endl;  
    std::cerr << "Done";
}