#pragma once

#include "rtweekend.h"

struct camera_settings {
    point3 lookfrom;
    point3 lookat;

    double aperture = 0;
    double vfov = 20.;

    vec3 vup{ 0, 1, 0 };
};

class camera {
public:
    __device__ camera(point3 lookfrom, point3 lookat, vec3 vup,
        double vfov, //vertical fov in degrees
        double aperture,
        double focus_dist,
        int nx,
        int ny
        ) : image_width(nx), image_height(ny), focus_dist(focus_dist)
     {
        //Camera orientation
        w = glm::normalize(lookfrom - lookat);
        u = glm::normalize(cross(vup, w));
        v = cross(w, u);

        //Camera Projection Plane
        auto theta = glm::radians(vfov);
        auto h = tan(theta / 2);
        const double viewport_height = 2.0*h;
        const double viewport_width = viewport_height * double(nx)/double(ny);

        lens_radius = aperture / 2;

        origin = lookfrom;
        horizontal = focus_dist*viewport_width * u;
        vertical = focus_dist*viewport_height * v;
        left_corner = origin - horizontal / 2.0 - vertical / 2.0 - w * focus_dist;
	}

    __device__ camera(camera_settings sett, int nx, int ny) : camera(
        sett.lookfrom,
        sett.lookat,
        sett.vup,
        sett.vfov,
        sett.aperture,
        glm::distance(sett.lookfrom, sett.lookat),
        nx,ny)
    {}

    __device__ void move(vec3 movement) {
        origin += u * movement.x + v * movement.y + w * movement.z;
        left_corner = origin - horizontal / 2.0 - vertical / 2.0 - w * focus_dist;
    }

    __device__ ray get_ray(curandState* rng, double s, double t) const {
        vec3 rd = lens_radius * random_in_unit_disk(rng);
        vec3 offset = u * rd.x + v * rd.y;
        return ray(origin+offset, left_corner + horizontal * s + vertical * t - origin-offset, white_wavelength);
    }
    __device__ ray get_mouse_ray(double s, double t) const {
        return ray(origin, left_corner + horizontal * s + vertical * t - origin, white_wavelength);
    }
private:
    vec3 origin;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    double lens_radius;
    double focus_dist;
    point3 left_corner;
public:
    //Image
    int image_width;
    int image_height;
};