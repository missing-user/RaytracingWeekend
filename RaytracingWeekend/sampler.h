#pragma once
#include "rtweekend.h"


/*
Sampling Methods
They return where the current pixel should be sampled, originating from the pixel center. The z coordinate does not have a meaning
*/
static vec3 sample_uniform(RNG& rng, int sample_index) {
	return rng.random()-vec3(.5,.5,0.);
}

static vec3 sample_center(RNG& rng, int sample_index) {
	return vec3();
}

static vec3 sample_normal(RNG& rng, int sample_index) {
	return vec3(rng.random_normal_double(), rng.random_normal_double(), 0.);
}

static vec3 sample_grid(RNG& rng, int sample_index) {
	return vec3((sample_index % 10) / 10.-.5, (sample_index/10.)/10. - .5, 0.);
}

/*
Filter Methods
*/

static double box_filter(vec3 uv) {
	return 1.;
}

const double B = 1. / 3.;
const double C = 1. / 3.;
static double Mitchell1D(double x) {
	x = std::abs(2 * x);
	if (x > 1)
		return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
			(-12 * B - 48 * C) * x + (8 * B + 24 * C)) * (1.f / 6.f);
	else
		return ((12 - 9 * B - 6 * C) * x * x * x +
			(-18 + 12 * B + 6 * C) * x * x +
			(6 - 2 * B)) * (1.f / 6.f);
}

static double mitchell_filter(vec3 uv) {
	double weight = Mitchell1D(uv.x()) * Mitchell1D(uv.y());
	return 2. * weight;
}

static double gauss_filter(vec3 uv) {
	const double alpha = 8;
	const double k = std::exp(-alpha * .5 * .5) * std::exp(-alpha * .5 * .5);
	return std::exp(-alpha * uv.x() * uv.x()) * std::exp(-alpha * uv.x() * uv.x()) - k;
}

/*
Final combined method
*/

const auto filter = mitchell_filter;
const auto sampler = sample_uniform;

static vec3 sample_pixel(RNG& rng, int x, int y, int width, int height, int sample_index) {
	const vec3 offset = sampler(rng, sample_index);
	return vec3((x + .5 + offset.x()) / (width - 1.), (y + .5 + offset.y()) / (height - 1.), filter(offset));
}