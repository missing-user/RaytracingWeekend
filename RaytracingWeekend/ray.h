#pragma once
#include "rtweekend.h"

class ray {
private:
	vec3 orig;
	double wavelength;
	vec3 dir;
public:
	ray() : wavelength(white_wavelength) {};
	ray(const ray& r, double lambda) : orig(r.orig), dir(r.dir), wavelength(lambda) {}
	ray(const point3& origin, const vec3& direction, double lambda) : orig(origin), dir(direction), wavelength(lambda) {}

	point3 origin() const { return orig; }
	vec3 direction() const { return dir; }
	vec3 invdir() const { return 1./dir; }
	double lambda() const { return wavelength; }

	point3 at(double t) const {
		return orig + dir * t;
	}
};
