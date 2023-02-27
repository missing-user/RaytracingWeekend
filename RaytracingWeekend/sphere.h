#pragma once
#include "hittable.h"

class sphere : public hittable
{
public:
    __device__ sphere() : radius(0.), center({ 0,0,0 }) {};
    __device__ sphere(point3 cen, double r, material* m) : center(cen), radius(r), mat_ptr(m) {};

    __device__ virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
    __device__ virtual bool bounding_box(aabb& output_box) const override;

public:
    point3 center;
    double radius;
    material* mat_ptr;
};

__device__ bool sphere::bounding_box(aabb& output_box) const {
    output_box = aabb(
        center - vec3(std::fabs(radius), std::fabs(radius), std::fabs(radius)),
        center + vec3(std::fabs(radius), std::fabs(radius), std::fabs(radius))
    );
    return true;
}

__device__ bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    const vec3 oc = r.origin() - center;
    const double a = glm::length2(r.direction());
    const double half_b = dot(r.direction(), oc);
    const double c = glm::length2(oc) - radius * radius;

    const double discriminant = half_b * half_b - a * c;
    if (discriminant < 0) 
        return false;

    double root = (-half_b - sqrt(discriminant)) / a;
    if (root < t_min || root > t_max) {
        root = (-half_b + sqrt(discriminant)) / a;
        if (root < t_min || root > t_max) {
            return false;
        }
    }

    rec.t = root;
    rec.p = r.at(root);
    vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}