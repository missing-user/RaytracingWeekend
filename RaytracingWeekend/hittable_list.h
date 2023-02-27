#pragma once

#include "hittable.h"

#include <vector>
#include "aabb.h"

class hittable_list : public hittable {
public:
	__device__ hittable_list() {}
	__device__ hittable_list(hittable** l, int n) { objects = l; n = n; }
	__device__ void clear() {
		delete [] objects;
	}
	__device__ virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	__device__ virtual bool bounding_box(aabb& output_box) const override;

public:
	hittable** objects;
	size_t n;
};


__device__  bool hittable_list::bounding_box(aabb& output_box) const {
	if (objects == nullptr)
		return false;
	if (!objects[0]->bounding_box(output_box))
		return false;

	for (int i = 0; i<n; i++)
	{
		aabb tmp_box;
		if (objects[i]->bounding_box(tmp_box)) {
			output_box = surrounding_box(output_box, tmp_box);
		}
		else {
			return false;
		}
	}

	return true;
}

__device__ bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	hit_record tmp_rec;
	bool hit_anything=false;
	double closest_so_far = t_max;

	for (int i = 0; i < n; i++)
	{
		if (objects[i]->hit(r, t_min, closest_so_far, tmp_rec)) {
			hit_anything =true;
			closest_so_far = tmp_rec.t;
			rec = tmp_rec;
		}
	}

	return hit_anything;
}