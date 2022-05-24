#pragma once

#include <iostream>
#include "vec3.h"


void write_color(std::string& out, color color_pixel) {
    // for string building / writing in a separate thread
    // gamma correction for a gamma of 2
    auto r = sqrt(color_pixel.x());
    auto g = sqrt(color_pixel.y());
    auto b = sqrt(color_pixel.z());

    int ir = static_cast<int>(255.999 * clamp(r));
    int ig = static_cast<int>(255.999 * clamp(g));
    int ib = static_cast<int>(255.999 * clamp(b));

    out += std::to_string(ir) + " " + std::to_string(ig) + " " + std::to_string(ib) + "\n";
}

void write_color(std::ostream& out, color color_pixel) {
    // gamma correction for a gamma of 2
    auto r = sqrt(color_pixel.x());
    auto g = sqrt(color_pixel.y());
    auto b = sqrt(color_pixel.z());

    int ir = static_cast<int>(255.999 * clamp(r));
    int ig = static_cast<int>(255.999 * clamp(g));
    int ib = static_cast<int>(255.999 * clamp(b));

    out << ir << ' ' << ig << ' ' << ib << std::endl;
}

static color wavelength_to_color(double wavelength, double gamma = 0.8) {
    double R, G, B;
    if (wavelength >= 380 & wavelength <= 440) {
        double attenuation = 0.3 + 0.7 * (wavelength - 380) / (440 - 380);
        R = std::pow((-(wavelength - 440) / (440 - 380)) * attenuation, gamma);
        G = 0.0;
        B = std::pow(1.0 * attenuation, gamma);
    }
    else if (wavelength >= 440 & wavelength <= 490) {
        R = 0.0;
        G = std::pow((wavelength - 440) / (490 - 440), gamma);
        B = 1.0;
    }
    else if (wavelength >= 490 & wavelength <= 510) {
        R = 0.0;
        G = 1.0;
        B = std::pow(-(wavelength - 510) / (510 - 490), gamma);
    }
    else if (wavelength >= 510 & wavelength <= 580) {
        R = std::pow((wavelength - 510) / (580 - 510), gamma);
        G = 1.0;
        B = 0.0;
    }
    else if (wavelength >= 580 & wavelength <= 645) {
        R = 1.0;
        G = std::pow(-(wavelength - 645) / (645 - 580), gamma);
        B = 0.0;
    }
    else if (wavelength >= 645 & wavelength <= 750) {
        double attenuation = 0.3 + 0.7 * (750 - wavelength) / (750 - 645);
        R = std::pow(1.0 * attenuation, gamma);
        G = 0.0;
        B = 0.0;
    }
    else {
        R = 0.0;
        G = 0.0;
        B = 0.0;
    }
    return color(R, G, B);
}