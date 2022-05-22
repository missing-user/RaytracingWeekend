#pragma once
#include "rtweekend.h"
#include "hittable.h"

struct hit_record;

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
public:
	lambertian(const color& a): albedo(a){}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override{
		auto scatter_direction = rec.normal + random_unit_vector();

		if (scatter_direction.near_zero()) scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	}

private:
	color albedo;
};


class metal : public material {
public:
	metal(const color & a, double f): albedo(a), fuzz(f< 1? f:1){}
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
		attenuation = albedo;
		return dot(scattered.direction(), rec.normal) > 0;
	}
	color albedo;
	double fuzz;
};

class dielectric : public material {
public:
	dielectric(const color& a, double refractive_index) : albedo(a), ir(refractive_index) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		double refraction_ratio = rec.front_face ? (1 / ir) : ir;

		auto in_vec = unit_vector(r_in.direction());
		auto cos_theta = fmin(dot(-in_vec, rec.normal), 1);
		double sin_theta = sqrt(1 - cos_theta * cos_theta);

		bool cannot_refract = refraction_ratio * sin_theta > 1;
		vec3 direction;
		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) {
			direction = reflect(in_vec, rec.normal);
		}
		else {
			direction = refract(in_vec, rec.normal, refraction_ratio);
		}

		scattered = ray(rec.p, direction);
		attenuation = albedo;
		return true;
	}
	color albedo;
	double ir; //refractive index
private:
	static double reflectance(double cosine, double ref_idx) {
		//Schlicks approximation
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};