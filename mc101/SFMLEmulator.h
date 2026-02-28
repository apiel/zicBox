#pragma once

#include <SFML/Graphics.hpp>
#include "draw/drawMono.h"

template <int W, int H>
class SFMLEmulator {
    int scale;
    sf::RenderWindow window;
    DrawMono<W, H>& drawer;

public:
    SFMLEmulator(DrawMono<W, H>& d, int pixelScale = 3) 
        : drawer(d), scale(pixelScale),
          window(sf::VideoMode(W * pixelScale, H * pixelScale), "SH1107 Emulator") {
        window.setFramerateLimit(60);
    }

    bool isOpen() { return window.isOpen(); }

    void update() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        window.clear(sf::Color(30, 30, 30)); // Dark grey background

        // Draw the monochrome buffer
        sf::RectangleShape pixel(sf::Vector2f(scale - 1, scale - 1));
        pixel.setFillColor(sf::Color::White);

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                if (drawer.getPixel({x, y})) {
                    pixel.setPosition(x * scale, y * scale);
                    window.draw(pixel);
                }
            }
        }
        window.display();
    }
};
