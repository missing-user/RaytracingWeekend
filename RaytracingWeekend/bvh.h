#pragma once
#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include <algorithm>

//TODO: This implementation is very inefficient. The BVH should be stored in a vector and not as a recursive tree on the stack

class bvh_node : public hittable {
public:
    bvh_node() {};
    bvh_node(std::vector<shared_ptr<hittable>>::iterator, std::vector<shared_ptr<hittable>>::iterator);

    bvh_node(hittable_list& list)
        : bvh_node(list.objects.begin(), list.objects.end())
    {}

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(aabb& output_box) const override;

public:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb box;
};

bool bvh_node::bounding_box(aabb& output_box) const {
    output_box = box;
    return true;
}

bool bvh_node::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    if (!box.hit(r, t_min, t_max))
        return false;

    bool hit_left = left->hit(r, t_min, t_max, rec);
    bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}

inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
    aabb box_a;
    aabb box_b;

    if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
        throw("No bounding box in bvh_node constructor.\n");

    // Sort by centroid
    return box_a.min()[axis]+box_a.max()[axis] < box_b.min()[axis]+box_b.max()[axis]; 
}


inline double pair_axis_dist(std::pair<std::vector<shared_ptr<hittable>>::iterator, std::vector<shared_ptr<hittable>>::iterator> p, int axis) {
    aabb box_a;
    aabb box_b;
    auto a = *p.first;
    auto b = *p.second;

    if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
        throw("No bounding box in bvh_node constructor.\n");

    return box_b.max()[axis] - box_a.min()[axis];
}


bool box_x_compare(const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
    return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
    return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<hittable>& a, const shared_ptr<hittable>& b) {
    return box_compare(a, b, 2);
}

bvh_node::bvh_node(std::vector<shared_ptr<hittable>>::iterator start, std::vector<shared_ptr<hittable>>::iterator end) {
    // select the largest axis to split the bvh
    int axis;
    auto minmax_x = std::minmax_element(start, end, box_x_compare);
    auto minmax_y = std::minmax_element(start, end, box_y_compare);
    auto minmax_z = std::minmax_element(start, end, box_z_compare);

    if (pair_axis_dist(minmax_x, 0) > std::fmax(pair_axis_dist(minmax_y, 1), pair_axis_dist(minmax_z, 2))) {
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
        axis = random_int(0, 2);
    }
    auto comparator = (axis == 0) ? box_x_compare
        : (axis == 1) ? box_y_compare
        : box_z_compare;

    //how many objects does this node contain?
    size_t object_span = end - start;
    auto mid = start + object_span / 2;
    std::partial_sort(start, mid, end, comparator);

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
        left = make_shared<bvh_node>(start, mid);
        right = make_shared<bvh_node>(mid, end);
    }*/

    // Terminate subdivision at a certain point
    if (object_span <= 8) {
        left = make_shared<hittable_list>(start, mid - start);
        right = make_shared<hittable_list>(mid, end - mid);
    }else{
        left = make_shared<bvh_node>(start, mid);
        right = make_shared<bvh_node>(mid, end);
    }

    aabb box_left, box_right;

    if (!left->bounding_box(box_left)
        || !right->bounding_box(box_right)
        )
        std::cerr << "No bounding box in bvh_node constructor.\n";

    box = surrounding_box(box_left, box_right);
}