#pragma once
#include "hittable.h"

class sphere : public hittable
{
public:
    sphere() : radius(0.) {};
	sphere(point3 cen, double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
    virtual bool bounding_box(aabb& output_box) const override;

public:
	point3 center;
	double radius;
    shared_ptr<material> mat_ptr;
};

bool sphere::bounding_box(aabb& output_box) const {
    output_box = aabb(
        center - vec3(radius, radius, radius),
        center + vec3(radius, radius, radius)
        );
    return true;
}

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    const vec3 oc = r.origin() - center;
    const double a = r.direction().length_squared();
    const double half_b = dot(r.direction(), oc);
    const double c = oc.length_squared() - radius * radius;

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
    rec.mat_ptr = mat_ptr.get();

    return true;
}