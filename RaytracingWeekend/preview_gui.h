#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>

#include "rtweekend.h"
#include "raytracer.h"

#ifdef EXR_SUPPORT
#include "exr_writer.h"
#endif // EXR_SUPPORT


class preview_gui {
public:
    preview_gui(std::string filename, const int width, const int height) : filename(filename), width(width), height(height) {};

    static void pixels_to_tex(sf::Uint8 *out_uint_pixels, const threaded_renderer& renderer) {
        const int i_max = renderer.pixels.size();
        for (int i = 0; i < i_max; i++)
        {
            const auto r = sqrt(renderer.pixels[i_max - i - 1].x() );
            const auto g = sqrt(renderer.pixels[i_max - i - 1].y() );
            const auto b = sqrt(renderer.pixels[i_max - i - 1].z() );

            out_uint_pixels[4 * i + 0] = sf::Uint8(255.999 * clamp(r));
            out_uint_pixels[4 * i + 1] = sf::Uint8(255.999 * clamp(g));
            out_uint_pixels[4 * i + 2] = sf::Uint8(255.999 * clamp(b));
            out_uint_pixels[4 * i + 3] = 255u;
        }
    }
    
    int open_gui(const threaded_renderer& renderer) {
        sf::RenderWindow window(sf::VideoMode(width, height), "Raytracer",
            sf::Style::Default | sf::Style::Close);
        sf::Texture tex;
        sf::Sprite sprite;

        sf::Uint8* pixel_data = new sf::Uint8[width * height * 4];

        if (!tex.create(width, height)) {
            std::cerr << "Couldn't create texture!" << std::endl;
            return 1;
        }

        tex.setSmooth(false);
        sprite.setTexture(tex);



        bool finished_rendering = false;

        while (window.isOpen() && !finished_rendering) {
            sf::Event event;
            while (window.pollEvent(event) && !finished_rendering) {
                if (event.type == sf::Event::Closed) window.close();
            }

            pixels_to_tex(pixel_data, renderer);
            tex.update(pixel_data);
            window.clear();
            window.draw(sprite);
            window.display();

            if (renderer.finished()) {
                finished_rendering = true;
            }

            std::cerr<< "\rProgress: " << std::fixed << std::setprecision(1) << renderer.get_percentage() * 100 << "% " << std::flush;

            sf::sleep(sf::milliseconds(500));
        }

        tex.copyToImage().saveToFile(filename + ".png");
        std::cerr << "Saved image to "<< filename+".png" << std::endl;

#ifdef EXR_SUPPORT
        const std::string exr_path = filename + ".exr";
        write_exr_file(exr_path.c_str(), width, height, renderer.pixels);
#endif // EXR_SUPPORT

        return 0;
    }
private:
    const int width, height;
    std::string filename;
};