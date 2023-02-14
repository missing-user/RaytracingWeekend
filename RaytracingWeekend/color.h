#pragma once

#include <iostream>
#include "spectrum.h"


void write_color(std::string& out, color color_pixel) {
    // for string building / writing in a separate thread
    // gamma correction for a gamma of 2
    auto r = sqrt(color_pixel.x);
    auto g = sqrt(color_pixel.y);
    auto b = sqrt(color_pixel.z);

    int ir = static_cast<int>(255.999 * clamp(r));
    int ig = static_cast<int>(255.999 * clamp(g));
    int ib = static_cast<int>(255.999 * clamp(b));

    out += std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + "\n";
}

void write_color(std::ostream& out, color color_pixel) {
    // gamma correction for a gamma of 2
    auto r = sqrt(color_pixel.x);
    auto g = sqrt(color_pixel.y);
    auto b = sqrt(color_pixel.z);

    int ir = static_cast<int>(255.999 * clamp(r));
    int ig = static_cast<int>(255.999 * clamp(g));
    int ib = static_cast<int>(255.999 * clamp(b));

    out << ir << ' ' << ig << ' ' << ib << std::endl;
}