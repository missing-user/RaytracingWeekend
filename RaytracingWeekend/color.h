#pragma once

#include <iostream>
#include "vec3.h"


void write_color(std::string& out, color color_pixel, int sample_count=1) {
    double factor = 1.0 / (sample_count);

    // gamma correction for a gamma of 2
    auto r = sqrt(color_pixel.x() * factor);
    auto g = sqrt(color_pixel.y() * factor);
    auto b = sqrt(color_pixel.z() * factor);

    int ir = static_cast<int>(255.999 * clamp(r));
    int ig = static_cast<int>(255.999 * clamp(g));
    int ib = static_cast<int>(255.999 * clamp(b));

    out += std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + "\n";
}