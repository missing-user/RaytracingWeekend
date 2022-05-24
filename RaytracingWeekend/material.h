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

		scattered = ray(rec.p, scatter_direction, r_in.wavelength);
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
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.wavelength);
		attenuation = albedo;
		return dot(scattered.direction(), rec.normal) > 0;
	}
	color albedo;
	double fuzz;
};

class dielectric : public material {
public:
	dielectric(const color& a, double refractive_index, double blur=0.) : albedo(a), ri(refractive_index), blur(fabs(blur) < 1 ? blur : 1) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		
		double r_index = ri_at_lambda(ri, dispersion, r_in.lambda());
		double refraction_ratio = rec.front_face ? (1. / r_index) : r_index;

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
			if (blur > 0)
				direction += random() * blur;
		}
		attenuation = albedo;

		scattered = ray(rec.p, direction, r_in.wavelength);
		return true;
	}
	color albedo;
	double ri; // refractive index
	double blur;
	double dispersion = 0.044*1e3; // dispersion coefficient in micrometers
private:
	static double reflectance(double cosine, double ref_idx) {
		//Schlicks approximation
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}

	static double ri_at_lambda(double ri, double disp, double wavelength) {
		return ri + (disp / wavelength);
	}
};