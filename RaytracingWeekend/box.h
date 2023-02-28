#pragma once
#include "hittable.h"
#include "quad.h"


class box : public hittable {
public:
    box(const point3& p0, const point3& p1, shared_ptr<material> ptr) : _aabb(p0, p1), mat_ptr(ptr) {
        radius = (_aabb.max() - _aabb.min()) * 0.5;
    }

    bool bounding_box(aabb& output_box) const override {
        output_box = _aabb;
        return true;
    }

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        vec3 m = r.invdir(); // can precompute if traversing a set of aligned boxes
        vec3 n = m * (r.origin() - _aabb.center());   // can precompute if traversing a set of aligned boxes
        vec3 k = glm::abs(m) * radius;
        vec3 t1 = -n - k;
        vec3 t2 = -n + k;

        double tN = glm::max(glm::max(t1.x, t1.y), t1.z);
        double tF = glm::min(glm::min(t2.x, t2.y), t2.z);

        if (tN > tF || tF < 0.0) return false; // no intersection

        rec.front_face = (tN > 0.0);
        rec.normal = (tN > 0.0) ? step(vec3(tN), t1) : // ro ouside the box
                                  step(t2, vec3(tF));  // ro inside the box
        rec.normal *= -glm::sign(r.direction());

        // Setup the rest of the hit record
        rec.t = (tN > 0.0) ? tN : tF;
        rec.p = r.at(rec.t);
        rec.mat_ptr = mat_ptr.get();

        return (tN > 0.0);
    }

public:
    shared_ptr<material> mat_ptr;

private:
    aabb _aabb;
    vec3 radius;
};




class rotate_y : public hittable {
public:
    rotate_y(shared_ptr<hittable> p, double angle);

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    virtual bool bounding_box(aabb& output_box) const override {
        output_box = bbox;
        return hasbox;
    }

public:
    shared_ptr<hittable> ptr;
    bool hasbox;
    aabb bbox;
private:
    double sin_theta;
    double cos_theta;
};

rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p) {
    auto radians = glm::radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(bbox);

    point3 min(infinity, infinity, infinity);
    point3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i * bbox.max().x + (1 - i) * bbox.min().x;
                auto y = j * bbox.max().y + (1 - j) * bbox.min().y;
                auto z = k * bbox.max().z + (1 - k) * bbox.min().z;

                auto newx = cos_theta * x + sin_theta * z;
                auto newz = -sin_theta * x + cos_theta * z;

                vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = std::min(min[c], tester[c]);
                    max[c] = std::max(max[c], tester[c]);
                }
            }
        }
    }

    bbox = aabb(min, max);
}


bool rotate_y::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto origin = r.origin();
    auto direction = r.direction();

    origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
    origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

    direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
    direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

    ray rotated_r(origin, direction, r.lambda());

    if (!ptr->hit(rotated_r, t_min, t_max, rec))
        return false;

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
    p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

    normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
    normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}