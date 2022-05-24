#pragma once
#include <cmath>
#include <iostream>

using std::sqrt;

class vec3
{
public: 
	vec3(): e{0,0,0} {}
	vec3(double e0, double e1, double e2): e{e0,e1,e2} {}

	double x() const { return e[0]; }
	double y() const { return e[1]; }
	double z() const { return e[2]; }

	vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	double operator[](int i) const { return e[i]; }
	double& operator[](int i) { return e[i]; } 

	vec3& operator+=(const vec3 &v) {
		e[0] += v[0];
		e[1] += v[1];
		e[2] += v[2];
		return *this;
	}

	vec3& operator*=(const double t) {
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}

	vec3& operator/=(const double t) {
		return  *this *= 1/ t;
	}

	double length() const {
		return sqrt(length_squared());
	}

	double length_squared() const {
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}

	bool near_zero() const {
		const auto s = 1e-8;
		return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
	}


public: 
	double e[3];
};

using point3 = vec3;
using color = vec3;


// vec3 Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
	return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

inline static vec3 random(double min=0, double max=1) {
	return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

vec3 random_in_unit_sphere() {
	while (true) {
		auto p = random(-1,1);
		if (p.length() < 1)
			return p;
	}
}

vec3 random_in_unit_disk() {
	while (true) {
		auto p = vec3(random_double(-1, 1), random_double(-1,1), 0);
		if (p.length() < 1)
			return p;
	}
}

inline vec3 operator+(const vec3& u, const vec3& v) {
	return vec3(u[0] + v[0], u[1] + v[1], u[2] + v[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
	return vec3(u[0] - v[0], u[1] - v[1], u[2] - v[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
	return vec3(u[0] * v[0], u[1] * v[1], u[2] * v[2]);
}

inline vec3 operator*(double t, const vec3& v) {
	return vec3(t * v[0], t * v[1], t * v[2]);
}

inline vec3 operator*(const vec3& v, double t) {
	return t * v;
}

inline vec3 operator/(vec3 v, double t) {
	return (1 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
	return u[0] * v[0]
		+ u[1] * v[1]
		+ u[2] * v[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
	return vec3(u[1] * v[2] - u[2] * v[1],
		u[2] * v[0] - u[0] * v[2],
		u[0] * v[1] - u[1] * v[0]);
}

inline vec3 unit_vector(vec3 v) {
	return v / v.length();
}

vec3 random_unit_vector() {
	return unit_vector(random_in_unit_sphere());
}

vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n) * n;
}

vec3 refract(const vec3& incoming, const vec3& n, double etai_over_etat) {
	auto cos_theta = fmin(dot(-incoming, n), 1);
	vec3 r_out_perp = etai_over_etat * (incoming + cos_theta * n);
	vec3 r_out_parallel = -sqrt(fabs(1 - r_out_perp.length_squared()))*n;
	return r_out_parallel + r_out_perp;
}