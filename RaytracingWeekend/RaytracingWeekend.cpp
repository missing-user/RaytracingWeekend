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
    camera_settings camset{ {13, 6, 0 }, {0,0,0} };
    camera cam(camset, 720);

    //Render
    threaded_renderer renderer(cam.image_width, cam.image_height, 32, 200, 32);
    preview_gui gui(filename, cam.image_width, cam.image_height);

    std::cerr << "Initializing Scene" << std::endl;

    // World
    hittable_list scene = random_scene();

    std::cerr << "Building BVH" << std::endl;
    auto bvh_scene = bvh_node(scene);

    gui.open_gui(renderer, bvh_scene, cam);

    const auto elapsed = std::chrono::high_resolution_clock::now() - start;

    std::cerr << "\nRender took: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()*1e-3 << "s\n";
    return 0;
}