#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>

#include "rtweekend.h"
#include "raytracer.h"

#ifdef EXR_SUPPORT
#include "exr_writer.h"
#endif // EXR_SUPPORT

struct interaction_state{
    vec3 movement;
    point3 click;
};

class preview_gui {
public:
    preview_gui(std::string filename, const int width, const int height) : filename(filename), width(width), height(height) {};

    static void pixels_to_tex(sf::Uint8 *out_uint_pixels, const threaded_renderer& renderer) {
        const int i_max = renderer.num_pixels;
        for (int i = 0; i < i_max; i++)
        {
            const auto r = sqrt(renderer.pixels[i_max - i - 1].x );
            const auto g = sqrt(renderer.pixels[i_max - i - 1].y );
            const auto b = sqrt(renderer.pixels[i_max - i - 1].z );

            out_uint_pixels[4 * i + 0] = sf::Uint8(255.999 * clamp(r));
            out_uint_pixels[4 * i + 1] = sf::Uint8(255.999 * clamp(g));
            out_uint_pixels[4 * i + 2] = sf::Uint8(255.999 * clamp(b));
            out_uint_pixels[4 * i + 3] = 255u;
        }
    }

    interaction_state get_input(sf::RenderWindow& window) {
        interaction_state out_s{};
        out_s.movement.x += sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        out_s.movement.x -= sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        out_s.movement.y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
        out_s.movement.y += sf::Keyboard::isKeyPressed(sf::Keyboard::E);
        out_s.movement.z -= sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        out_s.movement.z += sf::Keyboard::isKeyPressed(sf::Keyboard::S);

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i localPosition = sf::Mouse::getPosition(window);
            out_s.click = vec3(localPosition.x, localPosition.y, 0);
            out_s.click.x /= window.getSize().x;
            out_s.click.y /= window.getSize().y;
        }
        return out_s;
    }
    
    int open_gui(threaded_renderer& renderer, hittable** world, camera** cam) {
        renderer.render(world, cam);

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

        while (window.isOpen() && !renderer.finished()) {
            sf::Event event;
            while (window.pollEvent(event) && !renderer.finished()) {
                if (event.type == sf::Event::Closed) window.close();
            }

            pixels_to_tex(pixel_data, renderer);
            tex.update(pixel_data);
            window.clear();
            window.draw(sprite);
            window.display();

            /*if (renderer.finished()) {
                //finished_rendering = true;
                renderer.render(world, cam); 

                cam.move(get_input(window).movement);
                ray r = cam.get_mouse_ray(get_input(window).click.x, get_input(window).click.y);
                hit_record rec;
                if (world.hit(r, global_t_min, infinity, rec))
                    std::cerr << rec.p.x<<" "<<rec.p.y<<" "<<rec.p.z;
            }*/

            std::cerr << "\rProgress: " << std::fixed << std::setprecision(1) << renderer.get_percentage() * 100 << "% "<< std::flush;

            sf::sleep(sf::milliseconds(100));
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