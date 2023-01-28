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

std::string get_filename(int argc, char* argv[]) {
    std::string cmd = "out";
    for (int i = 1; i < argc; ++i)
    {
        cmd = argv[i];
    }

    return cmd;
}


int main(int argc, char* argv[])
{
    auto start = std::chrono::high_resolution_clock::now();
    std::cerr << "Initializing Renderer" << std::endl;

    //Render Settings
    const std::string filename = get_filename(argc, argv);

    //Camera Settings
    point3 lookfrom(13, 6, 0);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    double dist_to_focus = 10.0;
    auto aperture = 0.2;
    double vfov = 20.;

    dist_to_focus = 12.0;

    camera cam(lookfrom, lookat, vup, vfov, aperture, dist_to_focus, 720);

    //Render
    threaded_renderer renderer(cam.image_width, cam.image_height, 32, 1000, 50);
    preview_gui gui(filename, cam.image_width, cam.image_height);

    std::cerr << "Initializing Scene" << std::endl;

    // World
    hittable_list scene = random_scene();

    std::cerr << "Building BVH" << std::endl;
    auto bvh_scene = bvh_node(scene);

    renderer.render(bvh_scene, cam);
    gui.open_gui(renderer);
    const auto elapsed = std::chrono::high_resolution_clock::now() - start;

    std::cerr << std::endl << "Render took: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()*1e-3 << "s " << std::endl;
}