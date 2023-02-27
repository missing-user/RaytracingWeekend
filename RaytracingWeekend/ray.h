#pragma once
#include "rtweekend.h"

class ray {
private:
	vec3 orig;
	double wavelength;
	vec3 dir;
public:
	__device__ ray() : wavelength(white_wavelength) {};
	__device__ ray(const ray& r, double lambda) : orig(r.orig), dir(r.dir), wavelength(lambda) {}
	__device__ ray(const point3& origin, const vec3& direction, double lambda) : orig(origin), dir(direction), wavelength(lambda) {}

	__device__ point3 origin() const { return orig; }
	__device__ vec3 direction() const { return dir; }
	__device__ double lambda() const { return wavelength; }

	__device__ point3 at(double t) const {
		return orig + dir * t;
	}
};
