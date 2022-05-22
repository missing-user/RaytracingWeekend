#pragma once

#include "rtweekend.h"

class camera {
public:
    camera(point3 lookfrom, point3 lookat, vec3 vup,
        double vfov, //vertical fov in degrees
        double aspect,
        double aperture,
        double focus_dist
        ) 
     {
        //Camera orientation
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        //Camera Projection Plane
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        double viewport_height = 2.0*h;
        double viewport_width = viewport_height * aspect;

        lens_radius = aperture / 2;

        //Image
        image_width = 500;
        image_height = static_cast<int>(image_width / aspect);

        origin = lookfrom;
        horizontal = focus_dist*viewport_width * u;
        vertical = focus_dist*viewport_height * v;
        left_corner = origin - horizontal / 2 - vertical / 2 - w * focus_dist;
	}

    ray get_ray(double s, double t) const {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x() + v * rd.y();
        return ray(origin+offset, left_corner + horizontal * s + vertical * t - origin-offset);
    }
private:
    vec3 origin;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    double lens_radius;
    point3 left_corner;
public:
    int image_width;
    int image_height;
};