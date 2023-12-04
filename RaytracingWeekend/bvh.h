#pragma once
#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include <algorithm>
#include <functional>
#include <array>

//TODO: This implementation is very inefficient. The BVH should be stored in a vector and not as a recursive tree on the stack

class bvh_node : public hittable {
public:
    bvh_node() {};
    bvh_node(std::vector<shared_ptr<hittable>>::iterator, std::vector<shared_ptr<hittable>>::iterator);

    bvh_node(hittable_list& list)
        : bvh_node(list.begin(), list.end())
    {}

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(aabb& output_box) const override;
    void split_sah(std::vector<shared_ptr<hittable>>::iterator start, std::vector<shared_ptr<hittable>>::iterator end, int dim);

public:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb box;
};

bool bvh_node::bounding_box(aabb& output_box) const {
    output_box = aabb(box);
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

void bvh_node::split_sah(const std::vector<shared_ptr<hittable>>::iterator start, const std::vector<shared_ptr<hittable>>::iterator end, int dim) {
    struct BucketInfo {
        int count = 0;
        aabb bounds;
    };
    double minScore=std::numeric_limits<double>::max();
    int minDim=0;
    int min_cost_split;
    constexpr int num_bins = 24;

    for (dim = 0; dim < 3; dim++) {
        // Create bins to store the splitting cost for each possible split
       
        std::array<BucketInfo, num_bins> bins;


        // Assign the objects to the bins
        for (auto it = start; it < end; it++) {
            auto object = *it;
            aabb objbox{};
            object->bounding_box(objbox);

            // Get the bin index of the object
            int bin_index = (objbox.center()[dim] - box.min()[dim]) / (box.max()[dim] - box.min()[dim]) * num_bins;

            // Add the object to the bucket
            bins[bin_index].count++;
            if (bins[bin_index].count <= 1)
                bins[bin_index].bounds = objbox;
            else
                bins[bin_index].bounds = surrounding_box(bins[bin_index].bounds, objbox);
        }

        // Compute splitting costs
        std::array<double, num_bins - 1> cost;
        for (int i = 0; i < num_bins - 1; i++) {
            BucketInfo box0, box1;

            for (int j = 0; j <= i; j++) {
                if (bins[j].count > 0) {
                    if (box0.count == 0)
                        box0.bounds = bins[j].bounds;
                    else
                        box0.bounds = surrounding_box(box0.bounds, bins[j].bounds);
                    box0.count += bins[j].count;
                }
            }
            for (int j = i + 1; j < num_bins; j++) {
                if (bins[j].count > 0) {
                    if (box1.count == 0)
                        box1.bounds = bins[j].bounds;
                    else
                        box1.bounds = surrounding_box(box1.bounds, bins[j].bounds);
                    box1.count += bins[j].count;
                }
            }

            cost[i] = 0.125 + (box0.count * box0.bounds.surface_area() + box1.count * box1.bounds.surface_area()) / box.surface_area();
            //std::cout << "c=" << cost[i]<< "sa="<< bins[i].bounds.surface_area()<< ", ";
        }

        // Find the best split
        auto it = std::min_element(cost.begin(), cost.end());
        auto min_cost_itr = std::distance(cost.begin(), it);

        if (cost[min_cost_itr] < minScore) {
			minScore = cost[min_cost_itr];
			minDim = dim;

            min_cost_split = min_cost_itr;
		}
    }
    dim = minDim;



    //std::cout<< "Cost " << *it <<" at split "<< min_cost_itr << std::endl;
    std::vector<shared_ptr<hittable>>::iterator mid;
    // Split the list
    if (std::distance(start, end) > 8) { // Never allow for more than 8 objects in a leaf
        mid = std::partition(start, end, [min_cost_split, dim, this](const shared_ptr<hittable>& object) {
            aabb objbox;
            object->bounding_box(objbox);
            int bin_index = (objbox.center()[dim] - box.min()[dim]) / (box.max()[dim] - box.min()[dim]) * num_bins;
            return bin_index < min_cost_split;
        });
        if (mid == start)
            mid++;

    }else{
        // Split at the centroid of our bounding box
        std::sort(start, end, std::bind(box_compare, std::placeholders::_1, std::placeholders::_2, dim));
        mid = start + std::distance(start, end) / 2;
    }

    //std::cout << "Sort mid: " << std::distance(start, mid) << " of " << leaf_cost << "\n";

    if (std::distance(start, end) <= 0) {
        std::cout << "???" << std::distance(start, end);
        return;
    }
    else if (std::distance(start, end) == 1) {
        left = right = *start;
        return;
    }
    else if (std::distance(start, end) == 2) {
        left = *start;
        right = *(end - 1);
        return;
    }else {
        left = make_shared<bvh_node>(start, mid);
        right = make_shared<bvh_node>(mid, end);
    }
}


bvh_node::bvh_node(std::vector<shared_ptr<hittable>>::iterator start, std::vector<shared_ptr<hittable>>::iterator end) {
    // select the largest axis to split the bvh
    int axis;
    auto minmax_x = std::minmax_element(start, end, box_x_compare);
    auto minmax_y = std::minmax_element(start, end, box_y_compare);
    auto minmax_z = std::minmax_element(start, end, box_z_compare);

    (*start)->bounding_box(box);
    for (auto it = start; it < end; it++) {
        auto object = *it;
        aabb objbox{};
        object->bounding_box(objbox);
        box = surrounding_box(box, objbox);
    }

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

    split_sah(start, end, axis);

    aabb box_left, box_right;
    if (!left->bounding_box(box_left)
        || !right->bounding_box(box_right))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    box = surrounding_box(box_left, box_right);
}