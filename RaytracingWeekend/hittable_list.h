#pragma once

#include "hittable.h"

#include <vector>
#include "aabb.h"

class hittable_list : public hittable {
public:
	hittable_list() {}
	hittable_list(shared_ptr<hittable> object) { add(object); }

	void clear() {
		objects.clear();
	}
	void add(shared_ptr<hittable> object) { objects.push_back(object); }
	
	virtual bool hit(RNG& rng, const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;

public:
	std::vector<shared_ptr<hittable>> objects;
};


bool hittable_list::bounding_box(aabb& output_box) const {
	if (objects.empty())
		return false;
	if (!objects[0]->bounding_box(output_box))
		return false;

	for (const auto& object : objects)
	{
		aabb tmp_box;
		if (object->bounding_box(tmp_box)) {
			output_box = surrounding_box(output_box, tmp_box);
		}
		else {
			return false;
		}
	}

	return true;
}

bool hittable_list::hit(RNG& rng, const ray& r, double t_min, double t_max, hit_record& rec) const {
	hit_record tmp_rec;
	bool hit_anything=false;
	double closest_so_far = t_max;

	for (const auto& object : objects)
	{
		if (object -> hit(rng, r, t_min, closest_so_far, tmp_rec)) {
			hit_anything =true;
			closest_so_far = tmp_rec.t;
			rec = tmp_rec;
		}
	}

	return hit_anything;
}