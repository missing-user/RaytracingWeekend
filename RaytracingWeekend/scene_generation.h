#pragma once

#include "rtweekend.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"
#include "quad.h"
#include "box.h"
#include "obj_reader.h"

#include "bvh.h"


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


hittable_list glass_box_and_sphere() {
    hittable_list world;


    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));


    auto prismGlass = make_shared<dielectric>(color(1,1,1), 1.4, 0, 700);
    world.add(make_shared<sphere>(point3(1.5, .6, -1.6), .6, prismGlass));



    auto difflight = make_shared<diffuse_light>(color(20, 20, 20));
    world.add(make_shared<xy_rect>(1.2, 1.8, -1, 2, -.1, difflight));
    world.add(make_shared<sphere>(point3(-2, .2, 1.2), .1, difflight));

    auto green = make_shared<lambertian>(color(1, 1, 1));

    world.add(make_shared<box>(point3(0, -1, -1), point3(1.3, 1.8, 1), green));
    world.add(make_shared<box>(point3(1.7, -1, -1), point3(2, 1.8, 1), green));


    auto box2 = make_shared<box>(point3(0, 0, 2), point3(2, 2, 2.6), prismGlass);
    auto prism = make_shared<rotate_y>(box2, 30);
    world.add(prism);
    return world;
}


hittable_list caustics() {
    hittable_list world;


    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));


    auto prismGlass = make_shared<dielectric>(color(1,1,1), 1.41, 0);
    //world.add(make_shared<sphere>(point3(1.5, .6, -1.6), .6, prismGlass));
    auto iprismGlass = make_shared<dielectric>(color(1,1, 1), 1.51, 0);

    auto difflight = make_shared<diffuse_light>(color(50, 50, 50));
    world.add(make_shared<sphere>(point3(-2.2,2.2,.5),.2, difflight));

    auto glassObj = obj("susan2.obj", prismGlass);
    bvh_node mesh_node = bvh_node(glassObj);
    world.add(make_shared<bvh_node>(mesh_node));

    return world;
}


hittable_list horse_scene() {
    hittable_list world;

    auto green = make_shared<metal>(color(0, 1., 1.),0.0);
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


    auto material2 = make_shared<dielectric>(color(1, .8, .9), 1.5);

    auto mesh = obj("renderthis.obj", material2);
    bvh_node mesh_node = bvh_node(mesh);
    world.add(make_shared<bvh_node>(mesh_node));
    

    return world;
}



hittable_list final_scene() {
    hittable_list objects;
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            objects.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
        }
    }

    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(make_shared<sphere>(center1, 50, moving_sphere_material));

    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(260, 150, 45), 50, glass));
    objects.add(make_shared<sphere>(
        point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
        ));

    auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, glass);
    objects.add(boundary);

    auto emat = make_shared<lambertian>(color(0, 0, .8));
    objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
    objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(color(.6,.6,.6))));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(random(0, 165)+vec3(-100, 270, 395), 10, white));
    }

    return objects;
}