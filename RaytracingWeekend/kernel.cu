#include <iostream>
#include <string>
#include <chrono>

#include "preview_gui.h"

#include "sphere.h"
#include "raytracer.h"
#include "color.h"
#include "rtweekend.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"

__global__ void make_world(hittable** scene, camera **cam, int nx, int ny) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        //Camera Settings
        camera_settings camset{ {13, 6, 0 }, {0,0,0} };
        *cam = new camera(camset, nx, ny);
        *scene = new sphere(point3(2, 0, 0), 0.6, new lambertian(color(1, 0, 1)));
    }
}

int main() {
    const int res_y = 720;
    const int res_x = static_cast<int>(aspect_ratio * res_y);

    auto start = std::chrono::high_resolution_clock::now();
    std::cerr << "Initializing Renderer" << std::endl;
    //Render Settings
    const std::string filename = "out.png";

    //preview_gui gui(filename, cam.image_width, cam.image_height);

    std::cerr << "Initializing Scene" << std::endl;
    camera** cam;
    checkCudaErrors(cudaMalloc((void**)&cam, sizeof(camera*)));
    hittable** scene;
    checkCudaErrors(cudaMalloc((void**)&scene, sizeof(hittable*)));

    make_world << <1, 1 >> > (scene, cam, res_x, res_y);
    checkCudaErrors(cudaDeviceSynchronize());

    //Render
    threaded_renderer renderer{res_x, res_y};
    preview_gui gui{filename, res_x, res_y };
    gui.open_gui(renderer, scene, cam);

    const auto elapsed = std::chrono::high_resolution_clock::now() - start;
    /*for (int j = 0; j <= 720 - 1; j++) {
        for (int i = 0; i < aspect_ratio * 720; i++) {
            size_t pixel_index = j * aspect_ratio * 720 + i;
            std::cout << renderer.pixels[pixel_index].x << " " << renderer.pixels[pixel_index].y << " " << renderer.pixels[pixel_index].z << "\n";
        }
    }*/

    cudaDeviceReset();
}
