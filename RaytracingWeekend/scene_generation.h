#pragma once

#include "rtweekend.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

hittable_list random_scene() {
    /*
    
    //Camera Settings
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    double dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aperture, dist_to_focus, 720);

    //Render
    threaded_renderer renderer(cam.image_width, cam.image_height, 64, 100, 50);

    */
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random() * random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(color(1, 1, 1), 1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(color(.9, .4, 1), 1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));


    auto material2 = make_shared<lambertian>(color(5, 5, 5));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    auto material4 = make_shared<dielectric>(color(.9, 1, .5), 1.5, .2);
    world.add(make_shared<sphere>(point3(0, 1, 3), 1.0, material4));

    return world;
}


hittable_list random_disp() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random() * random();
                    sphere_material = make_shared<dielectric>(albedo, 1.3);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random(0.8, 1);
                    auto fuzz = random_double(.5, 1.5);
                    sphere_material = make_shared<dielectric>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(color(1, 1, 1), 1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material2 = make_shared<diffuse_light>(color(6,6,6));
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material2));

    auto material1 = make_shared<lambertian>(color(.9, 1, 1));
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material1));

    auto material3 = make_shared<metal>(color(1,1,1), 0.0);
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material3));

    auto material4 = make_shared<dielectric>(color(1, 1, 1), 1.5);
    world.add(make_shared<sphere>(point3(0, 1, 3), 1.0, material4));


    return world;
}

