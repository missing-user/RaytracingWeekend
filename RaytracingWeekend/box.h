#pragma once
#include "hittable.h"
#include "quad.h"

// These are indicies that are used in my method for Box::hit()
constexpr short Back = 0;
constexpr short Front = 1;
constexpr short Top = 2;
constexpr short Bottom = 3;
constexpr short Left = 4;
constexpr short Right = 5;

class box : public hittable {
public:
    box(const point3& p0, const point3& p1, shared_ptr<material> ptr) : _aabb(p0, p1), mat_ptr(ptr) {
        radius = (_aabb.max() - _aabb.min()) * 0.5;
        invRadius = 1.0 / radius;
    }

    bool bounding_box(aabb& output_box) const override {
        output_box = _aabb;
        return true;
    }

    bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        vec3 nrdir = normalize(r.direction());
        vec3 _invRayDir = 1.0 / nrdir;
        vec3 rorigin = r.origin() - _aabb.center();

        bool outsideBox = max(glm::abs(rorigin) * invRadius) >= 1.0;
        vec3 sgn = -sign(r.direction());

        // Distance to plane
        vec3 d = radius * (outsideBox ? 1. : -1.) * sgn - rorigin;
        d *= _invRayDir;

# define TEST(U, V, W) (d.U >= 0.0) && glm::all(glm::lessThan(glm::abs(glm::vec2(rorigin.V, rorigin.W) + glm::vec2(nrdir.V * d.U, nrdir.W * d.U) ), glm::vec2(radius.V, radius.W)))
        glm::bvec3 test{ TEST(x, y, z), TEST(y, z, x), TEST(z, x, y) };
        sgn = test.x ? vec3(sgn.x, 0, 0) : (test.y ? vec3(0, sgn.y, 0) : vec3(0, 0, test.z ? sgn.z : 0));
# undef TEST

        d *= r.direction();
        double distance = (sgn.x != 0) ? d.x : ((sgn.y != 0) ? d.y : d.z);
        // UV: box.invDirection * hitPoint
        distance *= glm::length(r.direction());

        // Setup the rest of the hit record
        rec.t = distance;
        rec.p = r.at(distance);
        rec.normal = sgn;
        rec.front_face = outsideBox;
        rec.mat_ptr = mat_ptr.get();

        return (sgn.x != 0) || (sgn.y != 0) || (sgn.z != 0);
    }

public:
    shared_ptr<material> mat_ptr;

private:
    aabb _aabb;
    vec3 invRadius;
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