#pragma once
#include "rtweekend.h"
#include "hittable.h"

class triangle : public hittable {
public:
	triangle(){}
	triangle(point3 p0, point3 p1, point3 p2, shared_ptr<material> m) :v0(p0), v1(p1), v2(p2), mat_ptr(m) {
        outward_normal = unit_vector(cross(v1-v0,v2-v0));
        v0v1 = v1 - v0;
        v0v2 = v2 - v0;

        //compute bounds
        point3 p_min = vmin(vmin(v0, v1), v2);
        point3 p_max = vmax(vmax(v0, v1), v2);

        precomputed_bounds = aabb(p_min, p_max);
	}

	virtual bool hit(RNG& rng, const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(aabb& output_box) const override;
public:
    shared_ptr<material> mat_ptr;

private:
    point3 v0, v1, v2;
    vec3 outward_normal;
    vec3 v0v1;
    vec3 v0v2;
    aabb precomputed_bounds;
};


//Möller Trumbore ray triangle intersection algorithm 
bool triangle::hit(RNG& rng, const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto pvec = cross(r.direction(), v0v2);
    double det = dot(v0v1, pvec);

    #ifdef CULLING 
        if (det < t_min) return false;
    #else 
        if (fabs(det) < t_min) return false; //parallel rays
    #endif 

    double iDeterminant = 1. / det;

    vec3 tvec = r.origin() - v0;
    double u = dot(tvec, pvec) * iDeterminant;
    if (u < 0 || u > 1) return false;

    vec3 qvec = cross(tvec, v0v1);
    double v = dot(r.direction(), qvec) * iDeterminant;
    if (v < 0 || u + v > 1) return false;

    rec.t = dot(v0v2, qvec) * iDeterminant;
    if (rec.t<t_min || rec.t > t_max)
        return false;

    rec.p = r.at(rec.t);
    //I'm not sure if the normals work correctly... maybe double check that
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr.get();

    return true;
}

bool triangle::bounding_box(aabb& output_box) const {
	output_box = precomputed_bounds;
	return true;
}

