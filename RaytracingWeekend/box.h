#pragma once
#include "hittable.h"
#include "quad.h"

// These are indicies that are used in my method for Box::hit()
constexpr size_t Back = 0;
constexpr size_t Front = 1;
constexpr size_t Top = 2;
constexpr size_t Bottom = 3;
constexpr size_t Left = 4;
constexpr size_t Right = 5;

class box : public hittable {
public:
    box() {}
    box(const point3& p0, const point3& p1, shared_ptr<material> ptr) : box_min(p0), box_max(p1), mat_ptr(ptr) {
        bounding_box(_aabb);
    }

    virtual bool bounding_box(aabb& output_box) const override {
        output_box = aabb(box_min, box_max);
        return true;
    }

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
        //Blatantly copied from https://github.com/define-private-public/PSRayTracing, adapted to fit my architecture

        // The `k` values that are used for `Rect` construction
        const double k[6]{
            box_max.z(),         // Back
            box_min.z(),         // Front
            box_max.y(),         // Top
            box_min.y(),         // Bottom
            box_max.x(),         // Left
            box_min.x()          // Right
        };

        const double t[6]{
            (k[Back] - r.origin().z()) / r.direction().z(),
            (k[Front] - r.origin().z()) / r.direction().z(),
            (k[Top] - r.origin().y()) / r.direction().y(),
            (k[Bottom] - r.origin().y()) / r.direction().y(),
            (k[Left] - r.origin().x()) / r.direction().x(),
            (k[Right] - r.origin().x()) / r.direction().x()
        };

        const double x[6]{
            r.origin().x() + (t[Back] * r.direction().x()),
            r.origin().x() + (t[Front] * r.direction().x()),
            r.origin().x() + (t[Top] * r.direction().x()),
            r.origin().x() + (t[Bottom] * r.direction().x()),
            0,
            0
        };

        const double y[6]{
            r.origin().y() + (t[Back] * r.direction().y()),
            r.origin().y() + (t[Front] * r.direction().y()),
            0,
            0,
            r.origin().y() + (t[Left] * r.direction().y()),
            r.origin().y() + (t[Right] * r.direction().y())
        };

        const double z[6]{
            0,
            0,
            r.origin().z() + (t[Top] * r.direction().z()),
            r.origin().z() + (t[Bottom] * r.direction().z()),
            r.origin().z() + (t[Left] * r.direction().z()),
            r.origin().z() + (t[Right] * r.direction().z())
        };

        const bool did_hit[6]{
            (t[Back] > t_min) && (t[Back] < t_max) && (x[Back] > box_min.x()) && (x[Back] < box_max.x()) && (y[Back] > box_min.y()) && (y[Back] < box_max.y()),
            (t[Front] > t_min) && (t[Front] < t_max) && (x[Front] > box_min.x()) && (x[Front] < box_max.x()) && (y[Front] > box_min.y()) && (y[Front] < box_max.y()),
            (t[Top] > t_min) && (t[Top] < t_max) && (x[Top] > box_min.x()) && (x[Top] < box_max.x()) && (z[Top] > box_min.z()) && (z[Top] < box_max.z()),
            (t[Bottom] > t_min) && (t[Bottom] < t_max) && (x[Bottom] > box_min.x()) && (x[Bottom] < box_max.x()) && (z[Bottom] > box_min.z()) && (z[Bottom] < box_max.z()),
            (t[Left] > t_min) && (t[Left] < t_max) && (y[Left] > box_min.y()) && (y[Left] < box_max.y()) && (z[Left] > box_min.z()) && (z[Left] < box_max.z()),
            (t[Right] > t_min) && (t[Right] < t_max) && (y[Right] > box_min.y()) && (y[Right] < box_max.y()) && (z[Right] > box_min.z()) && (z[Right] < box_max.z()),
        };

        // First see if any of them hit
        const bool any_hit = did_hit[Back] || did_hit[Front] || did_hit[Top] || did_hit[Bottom] || did_hit[Left] || did_hit[Right];
        if (!any_hit)
            return false;

        // Create an array,  If that side was hit, use it's `t` if not, set that to infinity.
        //   then we use `std::min()` to find the miniumum `t` (i.e. closest) value.
        using IndexedHit = std::pair<double, size_t>;
        const IndexedHit nearest = std::min({
                std::make_pair((did_hit[Back] ? t[Back] : infinity), Back),
                std::make_pair((did_hit[Front] ? t[Front] : infinity), Front),
                std::make_pair((did_hit[Top] ? t[Top] : infinity), Top),
                std::make_pair((did_hit[Bottom] ? t[Bottom] : infinity), Bottom),
                std::make_pair((did_hit[Left] ? t[Left] : infinity), Left),
                std::make_pair((did_hit[Right] ? t[Right] : infinity), Right)
            },
            [](const IndexedHit& a, const IndexedHit& b) { return a.first < b.first; }
        );

        const double nearest_t = nearest.first;
        const size_t nearest_i = nearest.second;


        const vec3 face_normals[6]{
            vec3(0, 0, 1), vec3(0, 0, 1),           // Back & Front
            vec3(0, 1, 0), vec3(0, 1, 0),           // Top & Bottom
            vec3(1, 0, 0), vec3(1, 0, 0),           // Left & Right
        };

        // Setup the rest of the hit record
        rec.t = nearest_t;
        rec.p = r.at(nearest_t);
        rec.set_face_normal(r, face_normals[nearest_i]);
        rec.mat_ptr = mat_ptr.get();

        return true;

    }

public:
    point3 box_min;
    point3 box_max;
    shared_ptr<material> mat_ptr;

private:
    aabb _aabb;
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
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(bbox);

    point3 min(infinity, infinity, infinity);
    point3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i * bbox.max().x() + (1 - i) * bbox.min().x();
                auto y = j * bbox.max().y() + (1 - j) * bbox.min().y();
                auto z = k * bbox.max().z() + (1 - k) * bbox.min().z();

                auto newx = cos_theta * x + sin_theta * z;
                auto newz = -sin_theta * x + cos_theta * z;

                vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
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

    ray rotated_r(origin, direction);

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