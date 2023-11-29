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
    
    int open_gui(threaded_renderer& renderer, hittable& world, camera& cam) {
        renderer.render(world, cam);

        sf::RenderWindow window(sf::VideoMode(width, height), "Raytracer",
            sf::Style::Default | sf::Style::Close | sf::Style::Resize);
        sf::Texture tex;
        sf::Sprite sprite;

        sf::Uint8* pixel_data = new sf::Uint8[width * height * 4];

        if (!tex.create(width, height)) {
            std::cerr << "Couldn't create texture!" << std::endl;
            return 1;
        }

        tex.setSmooth(false);
        sprite.setTexture(tex);

        sf::View view;
        view.setSize( width, height );
        view.setCenter( view.getSize().x / 2, view.getSize().y / 2 );
        view = getLetterboxView( view, width, height );  

        bool finished_rendering = false;

        while (window.isOpen() && !finished_rendering) {
            sf::Event event;
            while (window.pollEvent(event) && !finished_rendering) {
                if (event.type == sf::Event::Closed) window.close();
                if (event.type == sf::Event::Resized) 
                    view = getLetterboxView( view, event.size.width, event.size.height );
            }
            pixels_to_tex(pixel_data, sf::Keyboard::isKeyPressed(sf::Keyboard::N) ? renderer.pixels_normal : renderer.pixels);
            tex.update(pixel_data);
            window.clear();
            window.setView(view); 
            window.draw(sprite);
            window.display();

            if (renderer.finished()) {
                finished_rendering = true;
                renderer.render(world, cam); 

                cam.move(get_input(window).movement);
                ray r = cam.get_mouse_ray(get_input(window).click.x, get_input(window).click.y);
                hit_record rec;
                if (world.hit(r, global_t_min, infinity, rec))
                    std::cerr << rec.p.x<<" "<<rec.p.y<<" "<<rec.p.z;
            }

            std::cerr << "\rProgress: " << std::fixed << std::setprecision(1) << renderer.get_percentage() * 100 << "% "<<finished_rendering<< std::flush;

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

    static void pixels_to_tex(sf::Uint8 *out_uint_pixels, const std::vector<color>& pixels) {
        const int i_max = pixels.size();
        for (int i = 0; i < i_max; i++)
        {
            const auto r = sqrt(pixels[i_max - i - 1].x );
            const auto g = sqrt(pixels[i_max - i - 1].y );
            const auto b = sqrt(pixels[i_max - i - 1].z );

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

    sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight) {

    // Compares the aspect ratio of the window to the aspect ratio of the view,
    // and sets the view's viewport accordingly in order to archieve a letterbox effect.
    // A new view (with a new viewport set) is returned.

    float windowRatio = windowWidth / (float) windowHeight;
    float viewRatio = view.getSize().x / (float) view.getSize().y;
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
        horizontalSpacing = false;

    // If horizontalSpacing is true, the black bars will appear on the left and right side.
    // Otherwise, the black bars will appear on the top and bottom.

    if (horizontalSpacing) {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }

    else {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport( sf::FloatRect(posX, posY, sizeX, sizeY) );

    return view;
}
private:
    const int width, height;
    std::string filename;
};