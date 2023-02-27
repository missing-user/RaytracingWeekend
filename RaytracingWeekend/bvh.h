#pragma once
#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include <algorithm>

class bvh_node : public hittable {
public:
    __device__ bvh_node() {};
    __device__ bvh_node(hittable**, size_t);
    //__device__ bvh_node(hittable_list& list) : bvh_node(list.objects, list.n) {}

    __device__ virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    __device__ virtual bool bounding_box(aabb& output_box) const override;

public:
    hittable* left;
    hittable* right;
    aabb box;
    curandState* local_rand_state;
};

__device__ bool bvh_node::bounding_box(aabb& output_box) const {
    output_box = box;
    return true;
}

__device__ bool bvh_node::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    if (!box.hit(r, t_min, t_max))
        return false;

    bool hit_left = left->hit(r, t_min, t_max, rec);
    bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

__device__ int box_compare(const hittable* a, const hittable* b, int axis) {
    aabb box_a;
    aabb box_b;

    if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
    {
        //   std::cerr << ("No bounding box in bvh_node constructor.\n");
    }

    // Sort by centroid
    return box_a.min()[axis] + box_a.max()[axis] - box_b.min()[axis] + box_b.max()[axis] ? -1 : 1;
}
__device__ int box_compare(const void* ah, const void* bh, int axis) {
    hittable* a = *(hittable**)ah;
    hittable* b = *(hittable**)bh;
    return box_compare(a, b, axis);
}


__device__ inline double pair_axis_dist(cuda::std::pair<hittable*, hittable*> p, int axis) {
    aabb box_a;
    aabb box_b;

    if (!p.first->bounding_box(box_a) || !p.second->bounding_box(box_b))
    {
    //    std::cerr << ("No bounding box in bvh_node constructor.\n");
    }

    return box_b.max()[axis] - box_a.min()[axis];
}


__device__ int box_x_compare(const void* a, const void* b) {
    return box_compare(a, b, 0);
}

__device__ int box_y_compare(const void* a, const void* b) {
    return box_compare(a, b, 1);
}

__device__ int box_z_compare(const void* a, const void* b) {
    return box_compare(a, b, 2);
}

__device__ bvh_node::bvh_node(hittable** start, size_t n) {
    // select the largest axis to split the bvh
    int axis;

    cuda::std::pair<hittable*, hittable*> minmax_x{ start[0],start[0] };
    cuda::std::pair<hittable*, hittable*> minmax_y{ start[0],start[0] };
    cuda::std::pair<hittable*, hittable*> minmax_z{ start[0],start[0] };
    for (size_t i = 0; i < n; i++) {
        auto elm = start[i];
        if (box_x_compare(elm, minmax_x.first) < 0)
            minmax_x.first = elm;
        if (box_y_compare(elm, minmax_y.first) < 0)
            minmax_y.first = elm;
        if (box_z_compare(elm, minmax_z.first) < 0)
            minmax_z.first = elm;
        
        if (box_x_compare(elm, minmax_x.second) > 0)
            minmax_x.second = elm;
        if (box_y_compare(elm, minmax_y.second) > 0)
            minmax_y.second = elm;
        if (box_z_compare(elm, minmax_z.second) > 0)
            minmax_z.second = elm;
    }

    if (pair_axis_dist(minmax_x, 0) > cuda::std::max(pair_axis_dist(minmax_y, 1), pair_axis_dist(minmax_z, 2))) {
        axis = 0;
    }
    else if (pair_axis_dist(minmax_y, 1) > pair_axis_dist(minmax_z, 2)) {
        axis = 1;
    }
    else if (pair_axis_dist(minmax_y, 1) < pair_axis_dist(minmax_z, 2)) {
        axis = 2;
    }
    else{
        // multiple axis have the same size, pick a random one
        axis = static_cast<int>(random_double(local_rand_state, 0., 2.));
    }
    auto comparator = (axis == 0) ? box_x_compare
        : (axis == 1) ? box_y_compare
        : box_z_compare;

    //how many objects does this node contain?
    size_t object_span = n;
    cuda::std::qsort(&start, n, sizeof(hittable*), comparator);

    /*if (object_span == 1) {
        left = right = *start;
    }
    else if (object_span == 2) {
        if (comparator(*start, *(start + 1))) {
            left = *start;
            right = *(start + 1);
        }
        else {
            left = *(start + 1);
            right = *start;
        }
    }
    else {
        left = new bvh_node(start, mid);
        right = new bvh_node(mid, end);
    }*/

    // Terminate subdivision at a certain point

    if (object_span == 1) {
        right = left = *start;
    }else if (object_span <= 2) {
        left = *start;
        right = *(start +1);
    }else{
        left = new bvh_node(start, object_span / 2);
        right = new bvh_node(start + object_span / 2, object_span / 2);
    }

    aabb box_left, box_right;

    if (!left->bounding_box(box_left)
        || !right->bounding_box(box_right)
        )
    {//std::cerr << "No bounding box in bvh_node constructor.\n";
    }
    box = surrounding_box(box_left, box_right);
}