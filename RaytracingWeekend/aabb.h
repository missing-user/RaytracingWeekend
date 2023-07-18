#pragma once
#include "hittable.h"

class aabb{
public:
    aabb() = default;
	aabb(const point3& min_extent, const point3& max_extent): minimum(min_extent),maximum(max_extent) {}

	point3 min() const { return minimum; }
	point3 max() const { return maximum; }
    point3 center() const { return (minimum + maximum) * 0.5; }
    vec3 extent() const { return (maximum - minimum)*0.5; }
    double surface_area() const {
        return 2. * (extent().x * extent().y + extent().x * extent().z + extent().y * extent().z);
    }

    inline bool hit(const ray& r, double t_min, double t_max) const {
        // Slightly More performant hit test 
        const auto invDir = r.invdir();
		auto t0 = (min() - r.origin())*invDir;
		auto t1 = (max() - r.origin())*invDir;
	
        const auto t_min_aabb = glm::min(t0,t1);
        const auto t_max_aabb = glm::max(t0,t1);

        t_min = std::max({ t_min, t_min_aabb[0], t_min_aabb[1], t_min_aabb[2] });
        t_max = std::min({ t_max, t_max_aabb[0], t_max_aabb[1], t_max_aabb[2] });

		return (t_min < t_max);
    }
private:
	point3 minimum;
	point3 maximum;
};



static aabb surrounding_box(const aabb& box0, const aabb& box1) {
    return aabb(
        glm::min(box0.min(), box1.min()), 
        glm::max(box0.max(), box1.max()));
}