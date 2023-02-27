#pragma once
#include "rtweekend.h"
#include "hittable.h"

class triangle : public hittable {
public:
    __device__ triangle(point3 p0, point3 p1, point3 p2, material* m) :v0(p0), v0v1(p1 - p0), v0v2(p2 - p0), mat_ptr(m) {
        outward_normal = glm::normalize(cross(v0v1, v0v2));
        //compute bounds
        point3 p_min = glm::min(glm::min(p0, p1), p2);
        point3 p_max = glm::max(glm::max(p0, p1), p2);

        precomputed_bounds = aabb(p_min, p_max);
	}
    __device__ triangle(point3 p0, point3 p1, point3 p2, vec3 normal, material* m) :triangle(p0,p1,p2,m){
        outward_normal = glm::normalize(normal);
    }

    __device__ virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
    __device__ virtual bool bounding_box(aabb& output_box) const override;
private:
    point3 v0;
    vec3 v0v1;
    vec3 v0v2;
    vec3 outward_normal;
    aabb precomputed_bounds;
public:
    material* mat_ptr;
};

// #define CULLING
//Möller Trumbore ray triangle intersection algorithm 
__device__ bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto pvec = cross(r.direction(), v0v2);
    double det = dot(v0v1, pvec);
    
    #ifdef CULLING 
    if (!rec.front_face) return false; // Hit backface -> cull
    #else 
    if (fabs(det) < t_min) return false; // parallel rays
    #endif 

    double iDeterminant = 1. / det;

    vec3 tvec = r.origin() - v0;
    double u = dot(tvec, pvec) * iDeterminant;
    if (u < 0 || u > 1) return false;

    vec3 qvec = cross(tvec, v0v1);
    double v = dot(r.direction(), qvec) * iDeterminant;
    if (v < 0 || u + v > 1) return false;

    rec.t = dot(v0v2, qvec) * iDeterminant;
    if (rec.t<t_min || rec.t > t_max) return false;

    rec.p = r.at(rec.t);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}

__device__ bool triangle::bounding_box(aabb& output_box) const {
	output_box = precomputed_bounds;
	return true;
}

