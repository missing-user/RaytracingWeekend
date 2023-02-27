#pragma once
#include "rtweekend.h"
#include "hittable.h"

struct hit_record;

class material {
public:
	__device__ virtual bool scatter(curandState* rng, const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
	__device__ virtual color emitted(const ray& r_in, const hit_record& rec) const {return color(0, 0, 0);}
};

class lambertian : public material {
public:
	__device__ lambertian(const color& a) : albedo(a) {}

	__device__ virtual bool scatter(curandState* rng, const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override{
		auto scatter_direction = rec.normal + random_unit_vector(rng);

		if (glm::all(glm::epsilonEqual(scatter_direction, vec3(0,0,0), global_t_min))) scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction, r_in.lambda());
		attenuation *= albedo;
		return true;
	}

private:
	color albedo;
};


class directional_light : public material {
public:
	__device__ directional_light(color c, double angle) : emit(c), max_scalar_product(-std::cos(glm::radians(angle))), albedo(color(1, 1, 1)) {}

	__device__ virtual bool scatter(curandState* rng, const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		auto scatter_direction = rec.normal + random_unit_vector(rng);
		if (glm::all(glm::epsilonEqual(scatter_direction, vec3(0, 0, 0), global_t_min))) scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction, r_in.lambda());
		attenuation *= albedo;
		return true;
	}

	__device__ virtual color emitted(const ray& r_in, const hit_record& rec) const override {
		const vec3 unit_direction = glm::normalize(r_in.direction());
		if (dot(unit_direction, rec.normal) < max_scalar_product) {
			return emit;
		}
		return color{0,0,0};
	}

public:
	color emit;
private:
	color albedo;
	double max_scalar_product;
};

class diffuse_light : public material {
public:
	__device__ diffuse_light(color c) : emit(c) {}

	__device__ virtual bool scatter(curandState* rng,const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		return false;
	}

	__device__ virtual color emitted(const ray& r_in, const hit_record& rec) const override {
		return emit;
	}

public:
	color emit;
};

class metal : public material {
public:
	__device__ metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}
	__device__ virtual bool scatter(curandState* rng,const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(rng), r_in.lambda());
		attenuation *= albedo;
		return dot(scattered.direction(), rec.normal) > 0;
	}
	color albedo;
	double fuzz;
};

class anisotropic : public material {
//useful for smoke and stuff, interesting material 
public:
	__device__ anisotropic(color a) : albedo(a), anisotropy(0) {}
	__device__ anisotropic(color a, double anisotropy) : albedo(a), anisotropy(anisotropy) {}

	__device__ virtual bool scatter(curandState* rng,const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		auto direction = random_in_unit_sphere(rng) + normalize(r_in.direction()) * anisotropy;
		if (glm::all(glm::epsilonEqual(direction, vec3(0, 0, 0), global_t_min))) direction = rec.normal;
		scattered = ray(rec.p, direction, r_in.lambda());
		attenuation *= albedo;
		return true;
	}

public:
	color albedo;
	double anisotropy;
};


class specular : public material {
public:
	__device__ specular(const color& a, double f) : albedo(a), fuzz(f) {}
	__device__ virtual bool scatter(curandState* rng,const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		vec3 scatter_direction;
		if (random_double(rng) < fuzz) {
			scatter_direction = rec.normal + random_unit_vector(rng);
			if (glm::all(glm::epsilonEqual(scatter_direction, vec3(0,0,0), global_t_min))) scatter_direction = rec.normal;
		}
		else
			scatter_direction  = glm::reflect(glm::normalize(r_in.direction()), rec.normal);
		scattered = ray(rec.p, scatter_direction, r_in.lambda());
		attenuation *= albedo;
		return dot(scattered.direction(), rec.normal) > 0;
	}
	color albedo;
	double fuzz;
};

class dielectric : public material {
public:
	__device__ dielectric(double refractive_index) : albedo(color(1, 1, 1)), ri(refractive_index), blur(0.), dispersion(0.044 * 1e3) {}
	__device__ dielectric(const color& a, double refractive_index, double blur = 0., double disp = 0.044 * 1e3) : albedo(a), ri(refractive_index), blur(blur), dispersion(disp) {}
	__device__ virtual bool scatter(curandState* rng, const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {

		#ifdef DISPERSION
		const double r_index = ri_at_lambda(ri, dispersion, r_in.lambda());
		#else
		const double r_index = ri;
		#endif

		const double refraction_ratio = rec.front_face ? (1. / r_index) : r_index;

		const auto in_vec = glm::normalize(r_in.direction());
		const auto cos_theta = fmin(dot(-in_vec, rec.normal), 1);
		const double sin_theta = sqrt(1 - cos_theta * cos_theta);

		const bool cannot_refract = refraction_ratio * sin_theta > 1;
		vec3 direction;
		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double(rng)) {
			direction = reflect(in_vec, rec.normal);
		}
		else {
			direction = refract(in_vec, rec.normal, refraction_ratio);
			direction += random(rng) * blur;
		}

#ifdef LAMBERT_BEER
		if (!rec.front_face) {
			// We hit a backface (the ray must have travelled through the object)
			auto ray_length = glm::distance(r_in.origin(), rec.p);
			attenuation *= exp(-ray_length * (color(1)-albedo));
		}
#else
		attenuation *= albedo;
#endif
		scattered = ray(rec.p, direction, r_in.lambda());
		return true;
	}
	color albedo;
	double ri; // refractive index
	double blur;
	double dispersion; // dispersion coefficient in nanometers
private:
	__device__ inline double fastpow5(double input) const {
		const double a = input * input;
		return a * a * input;
	}

	__device__ double reflectance(double cosine, double ref_idx) const {
		//Schlicks approximation
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;

		return r0 + (1 - r0) * fastpow5(1 - cosine);
	}

	__device__ double ri_at_lambda(double ri, double disp, double wavelength) const {
		return ri + (disp / wavelength);
	}
};

class thinfilm : public material {
public:
	__device__ thinfilm(const color& a, double t, double n, const material* underlying = nullptr) : albedo(a), thickness(t), n0(1), n1(n), n2(1), underlying(underlying) {

	}
	__device__ virtual bool scatter(curandState* rng,const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		const double cos0 = abs(dot(r_in.direction(), rec.normal));

		// compute the phase change term (constant)
		double d10 = (n1 > n0) ? 0 : pi;
		double d12 = (n1 > n2) ? 0 : pi;
		double delta = d10 + d12;

		double t=0; // transmitted light

		// now, compute cos1, the cosine of the reflected angle
		double sin1 = pow(n0 / n1, 2) * (1 - cos0 * cos0);
		double sin2 = pow(n0 / n2, 2) * (1 - cos0 * cos0);

		if (sin1 > 1 || sin2 > 1) t = 1; // total internal reflection
		else {
			double cos1 = sqrt(1 - sin1);

			// compute cos2, the cosine of the final transmitted angle, i.e. cos(theta_2)
			// we need this angle for the Fresnel terms at the bottom interface
			double cos2 = sqrt(1 - sin2);

			// get the reflection transmission amplitude Fresnel coefficients
			double alpha_s = rs(n1, n0, cos1, cos0) * rs(n1, n2, cos1, cos2); // rho_10 * rho_12 (s-polarized)
			double alpha_p = rp(n1, n0, cos1, cos0) * rp(n1, n2, cos1, cos2); // rho_10 * rho_12 (p-polarized)

			double beta_s = ts(n0, n1, cos0, cos1) * ts(n1, n2, cos1, cos2); // tau_01 * tau_12 (s-polarized)
			double beta_p = tp(n0, n1, cos0, cos1) * tp(n1, n2, cos1, cos2); // tau_01 * tau_12 (p-polarized)

			// compute the phase term (phi)
			double phi = (2 * pi / r_in.lambda()) * (2 * n1 * thickness * cos1) + delta;

			// finally, evaluate the transmitted intensity for the two possible polarizations
			double ts = pow(beta_s, 2) / (pow(alpha_s, 2) - 2 * alpha_s * cos(phi) + 1);
			double tp = pow(beta_p, 2) / (pow(alpha_p, 2) - 2 * alpha_p * cos(phi) + 1);

			// we need to take into account conservation of energy for transmission
			double beamRatio = (n2 * cos2) / (n0 * cos0);

			// calculate the average transmitted intensity (if you know the polarization distribution of your
			// light source, you should specify it here. if you don't, a 50%/50% average is generally used)
			t = beamRatio * (ts + tp) / 2;
		}

		if (random_double(rng) < t) 
		{
			auto scatterRes = true;
			if(underlying == nullptr)
			{
				// transmission through air
				attenuation *= albedo;
				scattered = ray(rec.p, r_in.direction(), r_in.lambda());
			}
			else {
				// transmission using the underlying material
				auto scatterRes = underlying->scatter(rng, r_in, rec, attenuation, scattered); 
				attenuation *= albedo;
			}
			return scatterRes;
		}
		else
		{ //reflection
			vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
			scattered = ray(rec.p, reflected, r_in.lambda());
			attenuation *= albedo;
			return true;
		}
	}
private:
	color albedo;
	double thickness;
	double n0, n1, n2;
	const material* underlying;
	__device__ double rs(double n1, double n2, double cosI, double cosT) const {
		return (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
	}

	/* Amplitude reflection coefficient (p-polarized) */
	__device__ double rp(double n1, double n2, double cosI, double cosT) const {
		return (n2 * cosI - n1 * cosT) / (n1 * cosT + n2 * cosI);
	}

	/* Amplitude transmission coefficient (s-polarized) */
	__device__ double ts(double n1, double n2, double cosI, double cosT) const {
		return 2 * n1 * cosI / (n1 * cosI + n2 * cosT);
	}

	/* Amplitude transmission coefficient (p-polarized) */
	__device__ double tp(double n1, double n2, double cosI, double cosT) const {
		return 2 * n1 * cosI / (n1 * cosT + n2 * cosI);
	}
};

class normal :public material {
private:
	double brightness, saturation;
public:
	__device__ normal(double saturation = 1) :saturation(saturation), brightness(0.5) {}
	__device__ bool scatter(curandState* rng, const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
		return false;
	}
	__device__ color emitted(const ray& r_in, const hit_record& rec) const override {
		const vec3 a = (saturation * rec.front_face?rec.normal:-rec.normal) + vec3(brightness);
		return color(a);
	}
};