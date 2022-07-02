#pragma once
#include "hittable.h"

class aabb{
public:
	aabb(){}
	aabb(const point3& a, const point3& b): minimum(a),maximum(b) {}

	point3 min() const { return minimum; }
	point3 max() const { return maximum; }

    inline bool hit(const ray& r, double t_min, double t_max) const {
        const vec3 orig = r.origin(); //this seems to speed up the calculation by 1-2% due to better memory locality
        for (int a = 0; a < 3; a++) {
            const auto invD = 1.0 / r.direction()[a];
            auto t0 = (min()[a] - orig[a]) * invD;
            auto t1 = (max()[a] - orig[a]) * invD;
            if (invD < 0.0)
                std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min)
                return false;
        }
        return true;
    }
public:
	point3 minimum;
	point3 maximum;
};



static aabb surrounding_box(const aabb box0, const aabb box1) {
    return aabb(
        vmin(box0.min(), box1.min()), 
        vmax(box0.max(), box1.max()));
}