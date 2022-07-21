#pragma once
#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include <algorithm>

//TODO: This implementation is very inefficient. The BVH should be stored in a vector and not as a recursive tree on the stack

class bvh_node : public hittable {
public:
    bvh_node() {};
    bvh_node(std::vector<shared_ptr<hittable>>& src_objects, size_t start, size_t end);

    bvh_node(
        const std::vector<shared_ptr<hittable>>& src_objects,
        size_t start, size_t end);

    bvh_node(hittable_list& list)
        : bvh_node(list.objects, 0, list.objects.size())
    {}
    bvh_node(const hittable_list& list)
        : bvh_node(list.objects, 0, list.objects.size())
    {}

    virtual bool hit(RNG& rng, const ray& r, double t_min, double t_max, hit_record& rec) const override;

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

bool bvh_node::hit(RNG& rng, const ray& r, double t_min, double t_max, hit_record& rec) const {
    if (!box.hit(r, t_min, t_max))
        return false;

    bool hit_left = left->hit(rng, r, t_min, t_max, rec);
    bool hit_right = right->hit(rng, r, t_min, hit_left ? rec.t : t_max, rec);

    return hit_left || hit_right;
}


inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
    aabb box_a;
    aabb box_b;

    if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    return box_a.min().e[axis] < box_b.min().e[axis];
}


inline double pair_axis_dist(const std::pair<std::vector<shared_ptr<hittable>>::iterator, std::vector<shared_ptr<hittable>>::iterator> p, int axis) {
    aabb box_a;
    aabb box_b;
    auto a = *p.first;
    auto b = *p.second;

    if (!a->bounding_box(box_a) || !b->bounding_box(box_b))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    return box_b.max().e[axis] - box_a.min().e[axis];
}


bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 2);
}

bvh_node::bvh_node(std::vector<shared_ptr<hittable>>& src_objects,
    size_t start, size_t end) {
    auto &objects = src_objects; // Create a modifiable array of the source scene objects


    // select the largest axis to split the bvh
    int axis;
    auto minmax_x = std::minmax_element(objects.begin() + start, objects.begin() + end, box_x_compare);
    auto minmax_y = std::minmax_element(objects.begin() + start, objects.begin() + end, box_y_compare);
    auto minmax_z = std::minmax_element(objects.begin() + start, objects.begin() + end, box_z_compare);

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
        axis = RNG::random_int(0, 2);
    }
    auto comparator = (axis == 0) ? box_x_compare
        : (axis == 1) ? box_y_compare
        : box_z_compare;

    //how many objects does this node contain?
    size_t object_span = end - start;

    if (object_span == 1) {
        left = right = objects[start];
    }
    else if (object_span == 2) {
        if (comparator(objects[start], objects[start + 1])) {
            left = objects[start];
            right = objects[start + 1];
        }
        else {
            left = objects[start + 1];
            right = objects[start];
        }
    }
    else {
        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        auto mid = start + object_span / 2;
        left = make_shared<bvh_node>(objects, start, mid);
        right = make_shared<bvh_node>(objects, mid, end);
    }

    aabb box_left, box_right;

    if (!left->bounding_box(box_left)
        || !right->bounding_box(box_right)
        )
        std::cerr << "No bounding box in bvh_node constructor.\n";

    box = surrounding_box(box_left, box_right);
}