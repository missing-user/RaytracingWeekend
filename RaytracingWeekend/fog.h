#pragma once
#include "rtweekend.h"
#include "hittable.h"
#include "material.h"

class fog :public hittable {
public:
	fog(shared_ptr<hittable> b, double density, color a):neg_inv_density(-1 / density), boundary(b), phase_function(std::make_shared<anisotropic>(a)) {
	
	}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override {return boundary->bounding_box(output_box);}
public:
	shared_ptr<hittable> boundary;
	shared_ptr<material> phase_function;
	double neg_inv_density;
};

bool fog::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	hit_record rec_enter, rec_exit;


	if (!boundary->hit(r, -infinity, infinity, rec_enter))
		return false;

	if (!boundary->hit(r, rec_enter.t + global_t_min, infinity, rec_exit))
		return false;


	if (rec_enter.t < t_min) rec_enter.t = t_min;
	if (rec_exit.t > t_max) rec_exit.t = t_max;


	if (rec_enter.t >= rec_exit.t)
		return false;

	if (rec_enter.t < 0)
		rec_enter.t = 0;

	const auto ray_length = r.direction().length();
	const auto distance_inside_boundary = (rec_exit.t - rec_enter.t) * ray_length;
	const auto hit_distance = neg_inv_density * log(random_double());

	if (hit_distance > distance_inside_boundary)
		return false;

	rec.t = rec_enter.t + hit_distance / ray_length;
	rec.p = r.at(rec.t);


	rec.normal = vec3(1, 0, 0);  // arbitrary
	rec.front_face = true;     // also arbitrary
	rec.mat_ptr = phase_function.get();

	return true;
}