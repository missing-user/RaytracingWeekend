#pragma once
#include "aabb.h"
#include "rtweekend.h"

class material;

struct hit_record {
	point3 p;
	vec3 normal;
	double t;
	bool front_face;
	material *mat_ptr;

	inline void set_face_normal(const ray& r, const vec3& outward_normal) {
		front_face = dot(r.direction(), outward_normal) < 0;
		if (front_face) {
			normal = outward_normal;
		}
		else {
			normal = -outward_normal;
		}
	}
};

class hittable {
public:
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(aabb& output_box) const = 0;
};