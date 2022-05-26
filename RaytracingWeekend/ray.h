#pragma once
#include "vec3.h"

class ray {
public:
	ray() {}
	ray(const ray& r, double lambda) : orig(r.orig), dir(r.dir), wavelength(lambda) {}
	ray(const point3& origin, const vec3 direction) : orig(origin), dir(direction), wavelength(white_wavelength){}
	ray(const point3& origin, const vec3 direction, double wavelength) : orig(origin), dir(direction), wavelength(wavelength) {}

	point3 orgin() const { return orig; }
	vec3 direction() const { return dir; }
	double lambda() const { return wavelength; }

	point3 at(double t) const {
		return orig + t * dir;
	}
public:
	point3 orig;
	vec3 dir;
	double wavelength;
};
